#include "pch.h"
#include "Room.h"
#include "Content/Object/BaseObject.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Content/Object/Actor/PlayerPawn.h"

/*---------
   Room
---------*/

// TEMP: 임시로 전역으로 생성, 추후 Service와 Thread와 맞게 구조 변경 필요
std::shared_ptr<Room> GRoom = std::make_shared<Room>(1);

Room::Room(RoomId roomId)
   : roomId_(roomId)
   , objectManager_(roomId)
{
}

Room::~Room()
{
   objectManager_.ClearAll();
   
   roomPlayers_.clear();
   pawnObjects_.clear();
   npcTickList_.clear();
}

bool Room::Join(PlayerId playerId, SessionId sessionId)
{
   SendBufferRef enterResBuffer;
   std::vector<SendBufferRef> spawnBuffersToNewPlayer;
   SendBufferRef spawnBufferToOthers;
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindBySessionId(sessionId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      std::lock_guard<std::mutex> lock(mutex_);   // 방에 플레이어가 입장/퇴장할 때마다 Lock을 잡는 구조 (샤딩 도입 전까지는 이 구조로 유지)
   
      if (playerId == 0 or sessionId == 0)      // 유효하지 않은 playerId 또는 sessionId
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it != roomPlayers_.end()) {
         return false;   // 이미 방에 존재하는 플레이어의 경우 실패 처리
      }
      
      RoomPlayer newPlayer;
      newPlayer.playerId = playerId;
      newPlayer.sessionId = sessionId;
      
      auto playerPawn = SpawnObject<PlayerPawn>(ObjectFlags::Replicable | ObjectFlags::Tickable);
      if (!playerPawn) {
         consoleLogger->Log(Color::Yellow, L"[Room] Failed to spawn PlayerPawn for playerId %u\n", playerId);
         return false;   // 플레이어 Pawn 생성 실패
      }
      playerPawn->SetPosition(Vector3{0.0f + static_cast<float>(playerId * 100), 0.0f, 0.0f});   // TEMP: 플레이어마다 x축으로 100씩 떨어뜨려서 스폰하기
      
      newPlayer.pawnObjectId = playerPawn->GetId();
      
      auto [insertIt, inserted]= roomPlayers_.emplace(playerId, std::move(newPlayer));
      if (not inserted) return false;
      
      auto& joinedPlayer = insertIt->second;
      
      {
         // 입장한 플레이어에게 방 스냅샷 전송
         se::room::S_RoomEnterRes res;
         {
            res.set_success(true);
            auto* snapshot = res.mutable_snapshot();
            snapshot->set_room_id(roomId_);
            
            for (auto& [exPlayerId, exPlayer] : roomPlayers_) {
               auto* roomPlayer = snapshot->add_players();
               auto* playerIdPtr = roomPlayer->mutable_player_id();
               playerIdPtr->set_value(exPlayerId);
               auto* entityIdPtr = roomPlayer->mutable_entity_id();
               entityIdPtr->set_value(exPlayer.pawnObjectId.value);
               roomPlayer->set_nickname("Player" + std::to_string(exPlayerId));   // TEMP
            }
            
            auto* myEntityId = res.mutable_my_entity_id();
            myEntityId->set_value(joinedPlayer.pawnObjectId.value);
         }
         
         enterResBuffer = ServerPacketHandler::MakeSendBuffer(res);
      }
      {
         // 입장한 플레이어에게 기존 플레이어들의 스폰 정보 전송
         for (const auto& [exPlayerId, exPlayer] : roomPlayers_) {
            
            if (exPlayerId == playerId)
               continue;   // 자기 자신은 제외
            
            se::room::N_EntitySpawn spawnPkt;
            {
               auto* spawnInfo = spawnPkt.mutable_info();
               
               spawnInfo->set_type(se::common::OBJ_PLAYER);
               spawnInfo->set_template_id(1);   // TEMP (기본 플레이어...?)
               auto* entityIdPtr = spawnInfo->mutable_entity_id();
               entityIdPtr->set_value(exPlayer.pawnObjectId.value);
               
               auto* movementPtr = spawnInfo->mutable_movement();
               auto* positionPtr = movementPtr->mutable_position();
               
               auto* exPawn = objectManager_.Find(exPlayer.pawnObjectId);
               if (!exPawn) {
	               consoleLogger->Log(Color::Yellow, L"[Room] PlayerPawn not exist\n");
                  continue;
               }
               
               auto* exPlayerPawn = dynamic_cast<PlayerPawn*>(exPawn);
               if (!exPlayerPawn) {
                  consoleLogger->Log(Color::Yellow, L"[Room] PlayerPawn is not PlayerPawn\n");
                  continue;
               }
               
               const auto& exPos = exPlayerPawn->GetPosition();
               positionPtr->set_x(exPos.x);
               positionPtr->set_y(exPos.y);
               positionPtr->set_z(exPos.z);
               
               movementPtr->set_yaw(exPlayerPawn->GetYaw());
               movementPtr->set_pitch(exPlayerPawn->GetPitch());
               
            }
            SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(spawnPkt);
            spawnBuffersToNewPlayer.push_back(sendBuffer);
         }
      }
      {
         se::room::N_EntitySpawn spawnPkt;
         {
            auto* spawnInfo = spawnPkt.mutable_info();
               
            spawnInfo->set_type(se::common::OBJ_PLAYER);
            spawnInfo->set_template_id(1);   // TEMP (기본 플레이어...?)
            auto* entityIdPtr = spawnInfo->mutable_entity_id();
            entityIdPtr->set_value(joinedPlayer.pawnObjectId.value);
            
            // TODO: 아래 SpawnPoint 부분을 나중에 Spawn 지점으로 변경하기 (미리 Spawn Point에 Pawn 값을 설정하기)
            auto* movementPtr = spawnInfo->mutable_movement();
            auto* positionPtr = movementPtr->mutable_position();
               
            const auto& JoinerPos = playerPawn->GetPosition();
            positionPtr->set_x(JoinerPos.x);
            positionPtr->set_y(JoinerPos.y);
            positionPtr->set_z(JoinerPos.z);
               
            movementPtr->set_yaw(playerPawn->GetYaw());
            movementPtr->set_pitch(playerPawn->GetPitch());
         }
         
         spawnBufferToOthers = ServerPacketHandler::MakeSendBuffer(spawnPkt);
      }
   }
   
   if (enterResBuffer)
      sessionRef->Send(enterResBuffer);   // 입장한 플레이어에게 방 스냅샷 전송
   for (const auto& buf : spawnBuffersToNewPlayer)
      sessionRef->Send(buf);   // 입장한 플레이어에게 기존 플레이어들의 스폰 정보 전송
   if (spawnBufferToOthers)
      Broadcast(spawnBufferToOthers);   // 입장한 플레이어를 spawn
   
   return true;
}

bool Room::Leave(PlayerId playerId)
{
   SendBufferRef leaveResBuffer;
   SendBufferRef despawnBufferToOthers;
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindByPlayerId(playerId);
   
   {
      // TEMP
      std::lock_guard<std::mutex> lock(mutex_);
      
      if (playerId == 0)
         return false;   // 유효하지 않은 playerId
   
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (sessionRef) 
      {
         se::room::S_RoomLeaveRes res;
         res.set_success(true);
         
         leaveResBuffer = ServerPacketHandler::MakeSendBuffer(res);
      }
      
      {
         se::room::N_EntityDespawn noti;
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         despawnBufferToOthers = ServerPacketHandler::MakeSendBuffer(noti);
      }
   
      const ObjectId pawnId = it->second.pawnObjectId;
      roomPlayers_.erase(it);
      
      if (pawnId != ObjectId{}) {
         DespawnObject(pawnId);   // 플레이어의 Pawn이 존재하면 제거
      }
   }
   
   if (sessionRef and leaveResBuffer)
      sessionRef->Send(leaveResBuffer);   // 퇴장한 플레이어에게 퇴장 결과 전송
   
   if (despawnBufferToOthers)
      Broadcast(despawnBufferToOthers, playerId);
   
   return true;
}

bool Room::UpdateSession(PlayerId playerId, SessionId newSessionId)
{
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);
   
   if (newSessionId == 0)
      return false;   // 유효하지 않은 sessionId
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;   // 방에 존재하지 않는 플레이어
   
   it->second.sessionId = newSessionId;
   return true;
}

bool Room::HandleLoadingComplete(PlayerId playerId)
{
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      // TEMP
      std::lock_guard<std::mutex> lock(mutex_);
      
      if (playerId == 0) 
         return false;   // 유효하지 않은 playerId
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;
      
      if (it->second.loadingComplete)
         return true;   // 이미 로딩 완료 처리된 플레이어
      
      it->second.loadingComplete = true;
   }
   
   return true;
}

bool Room::HandleMove(PlayerId playerId, const se::game::C_MoveReq& pkt)
{
   SendBufferRef moveBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   // TODO: 기본적으로 본인 Player에겐 예외, 다만 유효성 판정 실패 시 보정 패킷을 보내야 한다
   
   {
      // TEMP
      std::lock_guard<std::mutex> lock(mutex_);
      
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loadingComplete)
         return false;
      
      auto* obj = objectManager_.Find(it->second.pawnObjectId);
      if (!obj)
         return false;
      
      auto* playerPawn = dynamic_cast<PlayerPawn*>(obj);
      if (!playerPawn)
         return false;
      
      const auto& move = pkt.movement();
      const auto& pos = move.position();
      
      // TODO: 유효성 판정은 여기서
      //       bool 값으로 유효성 판정 결과를 받고, 유효하지 않은 경우 보정 패킷을 보내는 구조로 변경하기 (클라이언트와 서버의 위치가 달라지는 경우 보정 패킷을 보내는 구조로)
      
      playerPawn->SetPosition(Vector3{pos.x(), pos.y(), pos.z()});
      playerPawn->SetYaw(move.yaw());
      playerPawn->SetPitch(move.pitch());
      
      se::game::N_Move noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         auto* movementPtr = noti.mutable_movement();
         auto* positionPtr = movementPtr->mutable_position();
         
         const auto& newPos = playerPawn->GetPosition();
         positionPtr->set_x(newPos.x);
         positionPtr->set_y(newPos.y);
         positionPtr->set_z(newPos.z);
         
         movementPtr->set_yaw(playerPawn->GetYaw());
         movementPtr->set_pitch(playerPawn->GetPitch());
         movementPtr->set_speed(move.speed());
      }
      
      moveBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (moveBroadcastBuffer)
      Broadcast(moveBroadcastBuffer, playerId);   // 이동한 플레이어를 제외한 나머지 플레이어들에게 이동 정보 Broadcast
   
   return true;
}

// bool Room::HandleMove(PlayerId playerId, const se::room::C_MoveInput& pkt)
// {
//    ObjectId playerPawnId = GetObjectId(playerId);
//    
//    // TEMP
//    std::lock_guard<std::mutex> lock(mutex_);
//    
//    auto* obj = objectManager_.Find(playerPawnId);
//    if (not obj)
//       return false;   // 플레이어의 Pawn이 존재하지 않음
//    
//    auto* playerPawn = dynamic_cast<PlayerPawn*>(obj);
//    if (not playerPawn)
//       return false;   // 플레이어의 Pawn이 PlayerPawn이 아님 (이 경우은 발생하지 않아야 함)
//
//    const auto& entity = pkt.entity_state();
//    const auto& movement = entity.movement();
//    const auto& newPos = movement.position();
//    playerPawn->SetPosition(Vector3{ newPos.x(), newPos.y(), newPos.z() });
//    playerPawn->SetYaw(movement.yaw());
//    playerPawn->SetPitch(movement.pitch());
//    
//    {
//       // TODO: 나중엔 Replicated에서 Dirty 체크해서 필요한 정보만 보내도록 변경하기 (한 틱에 한번에) <- repeated 키워드를 적극 활용 하기 위해
//       se::room::S_EntityState entityStatePkt;
//       {
//          auto moveEntity = entityStatePkt.add_entities();
//       
//          moveEntity->CopyFrom(entity);
//       }
//       
//       SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(entityStatePkt);
//       Broadcast(sendBuffer, playerId);   // 이동한 플레이어를 제외한 나머지 플레이어들에게 이동 정보 Broadcast
//       // TODO: 만약 잘못된 이동 판정을 한다면 여기서 플레이어의 위치를 원래대로 되돌리는 패킷을 보내야할지도? (클라이언트와 서버의 위치가 달라지는 경우 보정 패킷을 보내는 구조로)
//    }
//    
//    return true;
// }

void Room::UpdateTick()
{
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);
   
   // Room 정책
   // NPC만 Tick 진행
   
   // NPC Tick
   for (size_t i = 0; i < npcTickList_.size();) {
       
      const ObjectId npcId = npcTickList_[i];
      
      BaseObject* npc = objectManager_.Find(npcId);
      
      if (not npc) {
         // NPC가 사라졌는데 Tick 리스트에 남아있는 경우, 리스트에서 제거
         npcTickList_[i] = npcTickList_.back();
         npcTickList_.pop_back();
         continue;
      }
      
      // TODO: NPC 업데이트 로직 구현하기 (예: AI 행동, 이동, 상태 변화 등)
      // npc->Update();
      
      ++i;
   }
   
   // objectManager_.SweepDestroy();   // 오브젝트 제거 처리
}

bool Room::HasPlayer(PlayerId playerId) const
{
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);
   
   return roomPlayers_.contains(playerId);
}

SessionId Room::GetSessionId(PlayerId playerId) const
{
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return 0;   // 방에 존재하지 않는 플레이어
   
   return it->second.sessionId;
}

ObjectId Room::GetObjectId(PlayerId playerId) const
{
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return ObjectId{};   // 방에 존재하지 않는 플레이어
   
   return it->second.pawnObjectId;
}

void Room::Broadcast(std::shared_ptr<SendBuffer> sendBuffer, PlayerId exceptPlayerId)
{
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);
   
   if (not sendBuffer)
      return;   // 유효하지 않은 SendBuffer
   
   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (playerId == exceptPlayerId)
         continue;   // 제외할 플레이어는 건너뛰기
      
      if (roomPlayer.sessionId == 0)
         continue;   // 유효하지 않은 세션 ID인 플레이어는 건너뛰기
      
      // TODO: 더 좋게 변경할 수 있다면 하기,,,
      //       현재는 Lock도 걸고 우아하지 않아 보임...
      g_SessionManager.FindBySessionId(roomPlayer.sessionId)->Send(sendBuffer);   // 세션을 찾아서 메시지 전송
   }
}

void Room::IndexObject_OnAdd(BaseObject* object)
{
   if (not object)
      return;   // 유효하지 않은 오브젝트
   
   const ObjectId objectId = object->GetId();
   
   if (dynamic_cast<Pawn*>(object)) {
      pawnObjects_.insert(objectId);
   }
   
   if (dynamic_cast<MonsterPawn*>(object)) {
      npcTickList_.push_back(objectId);
   }
}

void Room::IndexObject_OnRemove(ObjectId objectId)
{
   pawnObjects_.erase(objectId);
   
   auto it = std::find(npcTickList_.begin(), npcTickList_.end(), objectId);
   if (it != npcTickList_.end()) {
      *it = npcTickList_.back();
      npcTickList_.pop_back();
   }
   
   for (auto& [playerId, roomPlayer] : roomPlayers_) {
      if (roomPlayer.pawnObjectId == objectId) {
         roomPlayer.pawnObjectId = ObjectId{};   // Pawn이 제거된 경우, RoomPlayer의 Pawn Object ID 초기화
      }
   }
}

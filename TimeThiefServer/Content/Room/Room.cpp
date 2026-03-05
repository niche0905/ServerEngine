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
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);   // 방에 플레이어가 입장/퇴장할 때마다 Lock을 잡는 구조 (샤딩 도입 전까지는 이 구조로 유지)
   
   if (playerId == 0 or sessionId == 0)      // 유효하지 않은 playerId 또는 sessionId
      return false;
   
   auto it = roomPlayers_.find(playerId);
   if (it != roomPlayers_.end()) {
      it->second.sessionId = sessionId;
      return true;   // 이미 방에 존재하는 플레이어, 세션 정보만 업데이트
   }
   
   RoomPlayer newPlayer;
   newPlayer.playerId = playerId;
   newPlayer.sessionId = sessionId;
   
   auto playerPawn = SpawnObject<PlayerPawn>(ObjectFlags::Replicable | ObjectFlags::Tickable);
   newPlayer.pawnObjectId = playerPawn->GetId();
   
   roomPlayers_.emplace(playerId, std::move(newPlayer));
   
   // TODO: 여기서 생성 Broadcast를 해야한다... (mutex 변경이 필요할지도? <- recursive_mutex로)
   
   {
      se::room::N_EntitySpawn spawnPkt;
      {
         se::room::EntityState* entityState = spawnPkt.mutable_entity();
         
         se::common::EntityId* entityId = entityState->mutable_entity_id();
         entityId->set_value(playerPawn->GetId().value);
         se::common::MovementState* movementState = entityState->mutable_movement();
         se::common::Vector3* postion = movementState->mutable_position();
         postion->set_x(playerPawn->GetPosition().x);
         postion->set_y(playerPawn->GetPosition().y);
         postion->set_z(playerPawn->GetPosition().z);
         movementState->set_yaw(playerPawn->GetYaw());
         movementState->set_speed(0.0f);
         se::common::AimRotation* aimRotation = entityState->mutable_aim();
         aimRotation->set_pitch(playerPawn->GetPitch());
      }
      
      SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(spawnPkt);
      Broadcast(sendBuffer);
   }
   
   return true;
}

bool Room::Leave(PlayerId playerId)
{
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;   // 방에 존재하지 않는 플레이어
   
   const ObjectId pawnId = it->second.pawnObjectId;
   if (pawnId != ObjectId{}) {
      DespawnObject(pawnId);   // 플레이어의 Pawn이 존재하면 제거
   }
   
   roomPlayers_.erase(it);
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

bool Room::HandleMove(PlayerId playerId, const se::room::C_MoveInput& pkt)
{
   ObjectId playerPawnId = GetObjectId(playerId);
   
   // TEMP
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto* obj = objectManager_.Find(playerPawnId);
   if (not obj)
      return false;   // 플레이어의 Pawn이 존재하지 않음
   
   auto* playerPawn = dynamic_cast<PlayerPawn*>(obj);
   if (not playerPawn)
      return false;   // 플레이어의 Pawn이 PlayerPawn이 아님 (이 경우은 발생하지 않아야 함)

   const auto& entity = pkt.entity_state();
   const auto& movement = entity.movement();
   const auto& newPos = movement.position();
   playerPawn->SetPosition(Vector3{ newPos.x(), newPos.y(), newPos.z() });
   playerPawn->SetYaw(movement.yaw());
   playerPawn->SetPitch(entity.aim().pitch());
   
   {
      // TODO: 나중엔 Replicated에서 Dirty 체크해서 필요한 정보만 보내도록 변경하기 (한 틱에 한번에) <- repeated 키워드를 적극 활용 하기 위해
      se::room::S_EntityState entityStatePkt;
      {
         auto moveEntity = entityStatePkt.add_entities();
      
         moveEntity->CopyFrom(entity);
      }
      
      SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(entityStatePkt);
      Broadcast(sendBuffer, playerId);   // 이동한 플레이어를 제외한 나머지 플레이어들에게 이동 정보 Broadcast
      // TODO: 만약 잘못된 이동 판정을 한다면 여기서 플레이어의 위치를 원래대로 되돌리는 패킷을 보내야할지도? (클라이언트와 서버의 위치가 달라지는 경우 보정 패킷을 보내는 구조로)
   }
   
   return true;
}

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
   // // TEMP
   // std::lock_guard<std::mutex> lock(mutex_);
   
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

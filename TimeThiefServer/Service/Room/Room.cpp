#include "pch.h"
#include "Room.h"
#include "Content/Gameplay/Combat/PlayerCombatComponent.h"
#include "Content/Object/BaseObject.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Service/Player/PlayerManager/PlayerManager.h"

/*---------
   Room
---------*/

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

void Room::PostCreate()
{
   objectManager_.SetRoom(shared_from_this());
}

bool Room::Join(PlayerId playerId, SessionId sessionId)
{
   if (playerId == 0 or sessionId == 0)      // 유효하지 않은 playerId 또는 sessionId
      return false;
   
   SendBufferRef enterResBuffer;
   std::vector<SendBufferRef> spawnBuffersToNewPlayer;
   SendBufferRef spawnBufferToOthers;
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindBySessionId(sessionId);
   std::shared_ptr<Player> playerRef = g_PlayerManager.Find(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   if (!playerRef) return false;    // 플레이어가 존재하지 않음 (정상적이지 않은 상황)
   
   {
      std::lock_guard<std::recursive_mutex> lock(mutex_);   // 방에 플레이어가 입장/퇴장할 때마다 Lock을 잡는 구조 (샤딩 도입 전까지는 이 구조로 유지)
      
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
      playerRef->roomId_ = roomId_;    // 플레이어의 현재 방 ID 업데이트
      playerRef->pawnId_ = insertIt->second.pawnObjectId;
      
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
   std::shared_ptr<Player> playerRef = g_PlayerManager.Find(playerId);
   
   {
      // TEMP
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      
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
      playerRef->roomId_ = 0;    // 플레이어의 현재 방 ID 업데이트
      playerRef->pawnId_.value = 0;
      
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
   std::lock_guard<std::recursive_mutex> lock(mutex_);
   
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
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      
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
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      
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
         auto* velocityPtr = movementPtr->mutable_velocity();
         velocityPtr->set_x(move.velocity().x());
         velocityPtr->set_y(move.velocity().y());
         movementPtr->set_movement_mode(move.movement_mode());
      }
      
      moveBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (moveBroadcastBuffer)
      Broadcast(moveBroadcastBuffer, playerId);   // 이동한 플레이어를 제외한 나머지 플레이어들에게 이동 정보 Broadcast
   
   return true;
}

bool Room::HandleAim(PlayerId playerId, const se::game::C_AimReq& pkt)
{
   SendBufferRef aimBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      
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
      
      playerPawn->SetAiming(pkt.is_aiming());
      
      se::game::N_Aim aimNoti;
      {
         auto* entityIdPtr = aimNoti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         aimNoti.set_is_aiming(playerPawn->IsAiming());
      }
      
      aimBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(aimNoti);
   }
   
   if (aimBroadcastBuffer)
      Broadcast(aimBroadcastBuffer, playerId);   // 에임 상태를 변경한 플레이어를 제외한 나머지 플레이어들에게 에임 상태 변경 정보 Broadcast
   
   return true;
}

bool Room::HandleFire(PlayerId playerId, const se::game::C_FireReq& pkt)
{
   SendBufferRef fireBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      
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
      
      auto combatComp = playerPawn->GetCombatComponent();
      if (!combatComp) {
         consoleLogger->Log(Color::Yellow, L"[Room] PlayerPawn has no CombatComponent\n");
         return false;
      }
      
      AttackRequest attackReq;
      attackReq.type = pkt.weapon_id() != 3 ? AttackType::Hitscan : AttackType::Projectile;   // TEMP: weapon_id가 3이면 투사체, 아니면 히트스캔으로 간주하기 (나중에 Weapon Data로 관리하기)
      attackReq.instigator = playerPawn;
      const auto& startPos = pkt.start_position();
      attackReq.origin = Vector3{startPos.x(), startPos.y(), startPos.z()};
      const auto& dir = pkt.direction();
      attackReq.direction = Vector3{dir.x(), dir.y(), dir.z()};
      
      bool attackSucc = combatComp->TryAttack(attackReq);
      
      se::game::N_Fire noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         // TODO: pkt의 weapon_id를 바로 사용하지 않고 Player State의 Weapon Id 와 비교 한번 진행하기
         noti.set_weapon_id(pkt.weapon_id());
         
         auto* startPosPtr = noti.mutable_start_position();
         startPosPtr->set_x(startPos.x());
         startPosPtr->set_y(startPos.y());
         startPosPtr->set_z(startPos.z());
         
         auto* dirPtr = noti.mutable_direction();
         dirPtr->set_x(dir.x());
         dirPtr->set_y(dir.y());
         dirPtr->set_z(dir.z());
      }
      
      fireBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (fireBroadcastBuffer)
      Broadcast(fireBroadcastBuffer, playerId);   // 발사한 플레이어를 제외한 나머지 플레이어들에게 발사 정보 Broadcast
   
   return true;
}

bool Room::HandleThrowGrenade(PlayerId playerId, const se::game::C_ThrowGrenadeReq& pkt)
{
   SendBufferRef throwBroadcastBuffer;
   SendBufferRef grenadeSpawnBuffer;      // se::room::N_EntitySpawn 형태로 투척한 수류탄의 스폰 정보
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      
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
      
      const auto& startPos = pkt.start_position();
      const auto& dir = pkt.direction();
      
      // TODO: 투척 판정 및 폭발 처리 로직은 여기서 (예: GrenadePawn을 생성해서 투척, 일정 시간 후 폭발 처리 등)
      //       다음 Spawn? 로직도 여기서 진행 (Replicated가 붙은 Grenade Actor)
      //       우선 Inventory 유효성 체크도 해야함
      
      se::game::N_ThrowGrenade noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         noti.set_grenade_type(pkt.grenade_type());
         
         auto* startPosPtr = noti.mutable_start_position();
         startPosPtr->set_x(startPos.x());
         startPosPtr->set_y(startPos.y());
         startPosPtr->set_z(startPos.z());
         
         auto* dirPtr = noti.mutable_direction();
         dirPtr->set_x(dir.x());
         dirPtr->set_y(dir.y());
         dirPtr->set_z(dir.z());
      }
      
      throwBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (throwBroadcastBuffer)
      Broadcast(throwBroadcastBuffer, playerId);   // 투척한 플레이어를 제외한 나머지 플레이어들에게 투척 정보 Broadcast
   
   if (grenadeSpawnBuffer)
      Broadcast(grenadeSpawnBuffer);
   
   return true;
}

bool Room::HandleReload(PlayerId playerId, const se::game::C_ReloadReq& pkt)
{
   // TODO: 재장전 결과 패킷 (S_ReleadRes 를 작성해서 프로토콜 업데이트 하기)
   SendBufferRef reloadResultBuffer;      // 재장전 결과를 해당 플레이어에게 보내는 패킷 (예: 재장전 성공 여부, 남은 탄창 수 등)
   SendBufferRef reloadBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      
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
      
      auto* playerCombatComp = playerPawn->GetPlayerCombat();
      if (!playerCombatComp) {
         consoleLogger->Log(Color::Yellow, L"[Room] PlayerPawn has no PlayerCombatComponent\n");
         return false;
      }

      const uint32 handWeaponId = playerCombatComp->GetCurrentWeaponId();
      if (handWeaponId != pkt.weapon_id()) {
         consoleLogger->Log(Color::Yellow, L"[Room] Reload Failed: weapon_id mismatch (handWeaponId: %u, pkt.weapon_id: %u)\n", handWeaponId, pkt.weapon_id());
      }
      playerCombatComp->TryReload();
      
      se::game::N_Reload noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         noti.set_weapon_id(pkt.weapon_id());
      }
      
      reloadBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (reloadResultBuffer)
      sessionRef->Send(reloadResultBuffer);   // 재장전 결과를 해당 플레이어에게 전송
   
   if (reloadBroadcastBuffer)
      Broadcast(reloadBroadcastBuffer, playerId);   // 재장전한 플레이어를 제외한 나머지 플레이어들에게 재장전 정보 Broadcast
   
   return true;
}

bool Room::HandleWeaponChange(PlayerId playerId, const se::game::C_WeaponChangeReq& pkt)
{
   SendBufferRef weaponChangeBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = g_SessionManager.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      std::lock_guard<std::recursive_mutex> lock(mutex_);
      
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
      
      const uint32 weaponId = pkt.weapon_id();
      auto* playerCombatComp = playerPawn->GetPlayerCombat();
      if (!playerCombatComp) {
         consoleLogger->Log(Color::Yellow, L"[Room] PlayerPawn has no PlayerCombatComponent\n");
         return false;
      }
      playerCombatComp->SwitchWeapon(weaponId);
      
      const uint32 handWeaponId = playerCombatComp->GetCurrentWeaponId();
      se::game::N_WeaponChanged noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         noti.set_weapon_id(handWeaponId);
      }
      
      weaponChangeBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (weaponChangeBroadcastBuffer)
      Broadcast(weaponChangeBroadcastBuffer);   // 모두에게 무기 변경 정보 Broadcast (본인 플레이어에겐 잘못된 무기 교체 요구일 수도 있으므로 예외 없이 모두에게 Broadcast)
   
   return true;
}

// bool Room::HandleMove(PlayerId playerId, const se::room::C_MoveInput& pkt)
// {
//    ObjectId playerPawnId = GetObjectId(playerId);
//    
//    // TEMP
//    std::lock_guard<std::recursive_mutex> lock(mutex_);
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
   std::lock_guard<std::recursive_mutex> lock(mutex_);
   
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

bool Room::TraceHit(const SE::Physics::Ray& ray, SE::Physics::Hit::HitResult& outHit) const
{
   bool hasHit = false;
   float closestT = std::numeric_limits<float>::max();
   
   outHit.Reset();
   
   objectManager_.ForEachAlive([&](BaseObject* obj)
   {
      if (!obj)
         return;
      
      obj->ForEachCollider([&](ColliderComponent* collider)
      {
         if (!collider)
            return;
         
         const ColliderRole role = collider->GetRole();
         if (role != ColliderRole::Hit && role != ColliderRole::Hurtbox)
            return;   // 명중 판정이 필요한 콜라이더가 아닌 경우 건너 뛰기
         
         SE::Physics::RaycastHit rayHit{};
         if (!collider->GetCollider()->Raycast(ray, rayHit)) 
            return;
         
         if (!rayHit.hit)
            return;
         
         if (rayHit.t >= closestT) 
            return;
         
         outHit.hit = rayHit.hit;
         outHit.t = rayHit.t;
         outHit.point = rayHit.point;
         outHit.normal = rayHit.normal;
         
         outHit.group = SE::Physics::Hit::HitGroup::Torso;
         outHit.damageMultiplier = 1.0f;   // TODO: HitGroup에 따른 데미지 배율 적용하기
         outHit.partIndex = 0;   // TODO: HitGroup에 따른 부위 인덱스 적용하기
         outHit.actor = collider->GetOwnerActor();
         
         closestT = rayHit.t;
         hasHit = true;
      });
   });
   
   return hasHit;
}

bool Room::HasPlayer(PlayerId playerId) const
{
   // TEMP
   std::lock_guard<std::recursive_mutex> lock(mutex_);
   
   return roomPlayers_.contains(playerId);
}

SessionId Room::GetSessionId(PlayerId playerId) const
{
   // TEMP
   std::lock_guard<std::recursive_mutex> lock(mutex_);
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return 0;   // 방에 존재하지 않는 플레이어
   
   return it->second.sessionId;
}

ObjectId Room::GetObjectId(PlayerId playerId) const
{
   // TEMP
   std::lock_guard<std::recursive_mutex> lock(mutex_);
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return ObjectId{};   // 방에 존재하지 않는 플레이어
   
   return it->second.pawnObjectId;
}

void Room::Broadcast(std::shared_ptr<SendBuffer> sendBuffer, PlayerId exceptPlayerId)
{
   if (not sendBuffer)
      return;   // 유효하지 않은 SendBuffer

   // TEMP
   std::lock_guard<std::recursive_mutex> lock(mutex_);
   
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

bool Room::SendToPlayer(PlayerId playerId, SendBufferRef buffer)
{
   if (playerId == 0 or buffer == nullptr)
      return false;  // 유효하지 않은 playerId 또는 SendBuffer
   
   // THINK: 원래라면 Room에 있는 Container에 접근할 때 Lock을 하고 접근하는 것이 안전하다...
   //        다만... 현재 유일한 호출은 Room::HandleFire 여기의 Lock 안에서 호출 되므로 Dead lock이 발생한다...
   // // TEMP
   // std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;  // 방에 존재하지 않는 플레이어
   
   auto session = g_SessionManager.FindByPlayerId(playerId);
   if (!session)
      return false;  // 플레이어의 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   session->Send(buffer);
   return true;
}

void Room::BroadcastDeath(ObjectId objectId)
{
   se::game::N_EntityDied noti;
   {
      auto* entityIdPtr = noti.mutable_entity_id();
      entityIdPtr->set_value(objectId.value);
   }
   
   SendBufferRef deathBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   
   if (deathBroadcastBuffer)
      Broadcast(deathBroadcastBuffer);   // 모두에게 사망 정보 Broadcast
}

void Room::NotifyHealthChange(PlayerId id, int newHealth, int deltaHealth)
{
   se::game::N_HealthChanged noti;
   {
      noti.set_new_health(newHealth);
      noti.set_delta(deltaHealth);
   }
   
   SendBufferRef healthChangeBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   if (!healthChangeBuffer)
      return;   // 유효하지 않은 SendBuffer
   
   SendToPlayer(id, healthChangeBuffer);
}

void Room::BroadcastKillPlayer(ObjectId killerId, ObjectId victimId)
{
   se::game::N_KillPlayer noti;
   {
      auto* killerIdPtr = noti.mutable_killer_id();
      killerIdPtr->set_value(killerId.value);
      
      auto* victimIdPtr = noti.mutable_victim_id();
      victimIdPtr->set_value(victimId.value);
   }
   
   SendBufferRef killPlayerBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   if (killPlayerBuffer)
      Broadcast(killPlayerBuffer);
}

void Room::HandleDamageResult(Pawn* attacker, Actor* victim, const SE::Physics::Hit::HitResult& hitResult,
                              const DamageContext& ctx, const DamageResult& damageResult)
{
   if (!attacker || !victim)
      return;   // 유효하지 않은 공격자 또는 피해자
   
   // TODO: Hit 패킷 만들어지면 Room::BroadcastHit 작성하고 호출하기
   
   if (!damageResult.killed)
      return;
   
   const bool KillerIsPlayer = attacker->IsPlayer();
   const bool VictimIsPlayer = victim->IsPlayer();
   
   if (KillerIsPlayer and VictimIsPlayer) {
      const PlayerId killerPlayerId = attacker->GetOwnerPlayerId();
      
      Pawn* victimPawn = dynamic_cast<Pawn*>(victim);
      if (!victimPawn)
         return;  // 피해자가 Pawn이 아님 (이 경우는 발생하지 않아야 함)
      
      const PlayerId victimPlayerId = victimPawn->GetOwnerPlayerId();
      
      if (killerPlayerId == 0 or victimPlayerId == 0)
         return;   // 유효하지 않은 플레이어 ID (이 경우는 발생하지 않아야 함)
      
      // THINK: 2중 Lock 예상되는 부분...
      //        std::mutex를 reculsive 한 것으로 바꿀까..?
      if (!HasPlayer(killerPlayerId) || !HasPlayer(victimPlayerId))
         return;   // 방에 존재하지 않는 플레이어가 공격자 또는 피해자인 경우 (이 경우는 발생하지 않아야 함)
      
      const ObjectId killerId = attacker->GetId();
      const ObjectId victimId = victim->GetId();
      
      BroadcastKillPlayer(killerId, victimId);   // 모두에게 킬 정보 Broadcast
      return;
   }
   
   if (KillerIsPlayer and victim->IsNPC()) {
      // NPC Kill 처리...
      return;
   }
   
   if (attacker->IsNPC() and VictimIsPlayer) {
      // Player가 NPC에게 죽은 경우 처리...
      return;
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

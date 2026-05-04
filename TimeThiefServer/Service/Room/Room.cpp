#include "pch.h"
#include "Room.h"

#include <random>
#include <utility>
#include "Content/Gameplay/Combat/PlayerCombatComponent.h"
#include "Content/Object/BaseObject.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Content/Object/Actor/ProjectileActor.h"
#include "Content/Object/Actor/WorldItemActor.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Shard/GameShard.h"
#include "Content/Gameplay/Drop/DropTypes.h"
#include "Content/Object/Actor/ChestActor.h"
#include "Content/Object/Actor/StoreActor.h"
#include "Data/GameDataManager.h"

/*-----------------
   Local Helper
-----------------*/

namespace 
{
   void FillSpawnInfoBase(BaseObject* obj, uint32 templateId, se::room::SpawnInfo* outInfo);
   bool BuildPlayerSpawnInfo(PlayerPawn* playerPawn, se::room::SpawnInfo* outInfo);
   bool BuildMonsterSpawnInfo(MonsterPawn* monsterPawn, se::room::SpawnInfo* outInfo);
   bool BuildItemSpawnInfo(WorldItemActor* item, se::room::SpawnInfo* outInfo);
   bool BuildProjectileSpawnInfo(ProjectileActor* projectile, se::room::SpawnInfo* outInfo);
   bool BuildChestSpawnInfo(ChestActor* chest, se::room::SpawnInfo* outInfo);
   bool BuildStoreSpawnInfo(StoreActor* store, se::room::SpawnInfo* outInfo);
   
   bool BuildSpawnInfo(BaseObject* obj, se::room::SpawnInfo* outInfo)
   {
      if (!obj or !outInfo)
         return false;
      
      switch (obj->GetObjectType())
      {
      case ObjectType::OBJ_PLAYER:
         return BuildPlayerSpawnInfo(static_cast<PlayerPawn*>(obj), outInfo);
         
      case ObjectType::OBJ_MONSTER:
         return BuildMonsterSpawnInfo(static_cast<MonsterPawn*>(obj), outInfo);
         
      case ObjectType::OBJ_ITEM:
         return BuildItemSpawnInfo(static_cast<WorldItemActor*>(obj), outInfo);
         
      case ObjectType::OBJ_PROJECTILE:
         return BuildProjectileSpawnInfo(static_cast<ProjectileActor*>(obj), outInfo);
         
      case ObjectType::OBJ_CHEST:
         return BuildChestSpawnInfo(static_cast<ChestActor*>(obj), outInfo);
         
      case ObjectType::OBJ_STORE:
         return  BuildStoreSpawnInfo(static_cast<StoreActor*>(obj), outInfo);
         
      default:
         return false;
      }
   }
   
   void FillSpawnInfoBase(BaseObject* obj, uint32 templateId, se::room::SpawnInfo* outInfo)
   {
      outInfo->set_type(obj->GetObjectType());
      outInfo->set_template_id(templateId);
      outInfo->mutable_entity_id()->set_value(obj->GetId().value);
   }
   
   bool BuildPlayerSpawnInfo(PlayerPawn* playerPawn, se::room::SpawnInfo* outInfo)
   {
      if (!playerPawn or !outInfo)
         return false;
      
      FillSpawnInfoBase(playerPawn, 1, outInfo);   // TEMP: 플레이어는 templateId 1로 고정
      
      auto* detailPtr = outInfo->mutable_player_info();
      auto* movementPtr = detailPtr->mutable_movement();
      
      auto* posPtr = movementPtr->mutable_position();
      const SE::Math::Vector3& pos = playerPawn->GetPosition();
      posPtr->set_x(pos.x);
      posPtr->set_y(pos.y);
      posPtr->set_z(pos.z);
      
      movementPtr->set_yaw(playerPawn->GetYaw());
      movementPtr->set_pitch(playerPawn->GetPitch());
      
      // 초기 속도는 없음
      
      // 초기 movement move는 없음 (곧 바로 MoveReq가 올 것이라 생각)
      
      return true;
   }
   
   bool BuildMonsterSpawnInfo(MonsterPawn* monsterPawn, se::room::SpawnInfo* outInfo)
   {
      // TODO: 몬스터 필요한거 SpawnInfo 작성하기 (NPC 코드 부터 작성하고)
      consoleLogger->Log(Color::Yellow, L"[Room] You Don't have Monster Spawn Build\n");
      return false;
   }
   
   bool BuildItemSpawnInfo(WorldItemActor* item, se::room::SpawnInfo* outInfo)
   {
      if (!item or !outInfo)
         return false;
      
      // TODO: Template은 Item Id를 기반으로 지정하여야 함 (아니면 Item Info에 필드를 추가하는 방식)
      FillSpawnInfoBase(item, 1, outInfo);
      
      auto* detailPtr = outInfo->mutable_item_info();
      auto* posPtr = detailPtr->mutable_position();
      const SE::Math::Vector3& pos = item->GetPosition();
      posPtr->set_x(pos.x);
      posPtr->set_y(pos.y);
      posPtr->set_z(pos.z);
      auto* velocityPtr = detailPtr->mutable_velocity();
      const SE::Math::Vector3& velo = item->GetVelocity();
      velocityPtr->set_x(velo.x);
      velocityPtr->set_y(velo.y);
      velocityPtr->set_z(velo.z);
      detailPtr->set_amount(item->GetItemStack().count);
      
      return true;
   }
   
   bool BuildProjectileSpawnInfo(ProjectileActor* projectile, se::room::SpawnInfo* outInfo)
   {
      if (!projectile or !outInfo)
         return false;
      
      FillSpawnInfoBase(projectile, 1, outInfo);   // TODO: Projectile templateId 어떻게 할 지 고민 (Getter 하나 추가해도 좋고)
      
      auto* detailPtr = outInfo->mutable_projectile_info();
      auto* posPtr = detailPtr->mutable_position();
      const SE::Math::Vector3& pos = projectile->GetPosition();
      posPtr->set_x(pos.x);
      posPtr->set_y(pos.y);
      posPtr->set_z(pos.z);
      
      auto* velocityPtr = detailPtr->mutable_velocity();
      const SE::Math::Vector3& velo = projectile->GetVelocity();
      velocityPtr->set_x(velo.x);
      velocityPtr->set_y(velo.y);
      velocityPtr->set_z(velo.z);
      
      return true;
   }
   
   bool BuildChestSpawnInfo(ChestActor* chest, se::room::SpawnInfo* outInfo)
   {
      if (!chest or !outInfo)
         return false;
      
      FillSpawnInfoBase(chest, 1, outInfo);
      
      auto* detailPtr = outInfo->mutable_chest_info();
      auto* posPtr = detailPtr->mutable_position();
      const SE::Math::Vector3& pos = chest->GetPosition();
      posPtr->set_x(pos.x);
      posPtr->set_y(pos.y);
      posPtr->set_z(pos.z);
      
      const float yaw = chest->GetYaw();
      detailPtr->set_yaw(yaw);
      
      return true;
   }
   
   bool BuildStoreSpawnInfo(StoreActor* store, se::room::SpawnInfo* outInfo)
   {
      if (!store or !outInfo)
         return false;
      
      FillSpawnInfoBase(store, 1, outInfo);
      
      auto* detailPtr = outInfo->mutable_store_info();
      auto* posPtr = detailPtr->mutable_position();
      const SE::Math::Vector3& pos = store->GetPosition();
      posPtr->set_x(pos.x);
      posPtr->set_y(pos.y);
      posPtr->set_z(pos.z);
      
      const float yaw = store->GetYaw();
      detailPtr->set_yaw(yaw);
      
      return true;
   }
}

/*---------
   Room
---------*/

Room::Room(RoomId roomId, SessionManager& sessionManager)
   : roomId_(roomId)
   , objectManager_(roomId)
   , sessionManager_(sessionManager)
   , rng_(static_cast<uint32>(roomId))
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

bool Room::Init(GameShard* ownerShard, const GameDataManager& gameDataManager, const GameConfig& gameConfig)
{
   ownerShard_ = ownerShard;
   
   if (!roomGameSystem_.Init(this, gameDataManager, gameConfig))
      return false;
   
   gameDataManager_ = &gameDataManager;
   
   return true;
}

void Room::SetPlayer(const std::vector<PlayerId>& playerIds)
{
   roomPlayers_.clear();
   
   const size_t spawnPointCount = gameDataManager_->GetPlayerSpawnTable().spawnPoints.size();
   if (spawnPointCount < playerIds.size()) {
      consoleLogger->Log(Color::Red, L"[Room] Not enough spawn points for players. SpawnPointCount: %zu, PlayerCount: %zu\n", spawnPointCount, playerIds.size());
      return;   // 플레이어 수보다 스폰 포인트가 적은 경우 (정상적이지 않은 상황)
   }
   
   std::vector<Vector3> spawnPoints(gameDataManager_->GetPlayerSpawnTable().spawnPoints);
   
   std::random_device rd;
   std::mt19937 rng(rd());
   std::ranges::shuffle(spawnPoints, rng);
   
   for (size_t i = 0; i < playerIds.size(); ++i) {
      const PlayerId playerId = playerIds[i];

      RoomPlayer roomPlayer;
      roomPlayer.playerId = playerId;

      auto playerPawn = CreatePreparedPlayerPawn(playerId, spawnPoints[i]);
      if (!playerPawn) {
         consoleLogger->Log(Color::Yellow, L"[Room] Failed to pre-spawn PlayerPawn for playerId %u\n", playerId);
         continue;   // PlayerPawn 생성 실패한 경우 (정상적이지 않은 상황)
      }

      roomPlayer.pawnObjectId = playerPawn->GetId();

      roomPlayers_.emplace(playerId, std::move(roomPlayer));
   }
}

void Room::SetObject()
{
   // TODO: 변경하기 (기본 Spawn 정보 기반 Spawn)
   
   for (int32 i = 0; i < 5; ++i) {
      auto* chest = SpawnObject<ChestActor>(ObjectFlags::None);
      chest->SetPosition(Vector3{ 200.0f + i * 150.0f, 0.0f, 150.0f });
   }
}

bool Room::Join(PlayerId playerId, SessionId sessionId)
{
   if (playerId == 0 or sessionId == 0)      // 유효하지 않은 playerId 또는 sessionId
      return false;
   
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindBySessionId(sessionId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end()) {
      return false;   // 예약 되지 않은 플레이어의 경우 입장 실패 (정상적이지 않은 상황)
   }
      
   RoomPlayer& newPlayer = it->second;
      
   if (newPlayer.joined) {
      return false;   // 이미 입장한 플레이어가 다시 입장 시도 (정상적이지 않은 상황)
   }
      
   if (newPlayer.pawnObjectId == ObjectId{})
      return false;
   
   auto* playerPawn = objectManager_.FindAs<PlayerPawn>(newPlayer.pawnObjectId);
   if (!playerPawn) {
      consoleLogger->Log(Color::Yellow, L"[Room] PlayerPawn not exist for playerId %u\n", playerId);
      return false;   // 예약된 플레이어의 Pawn이 존재하지 않음 (정상적이지 않은 상황)
   }
   
   newPlayer.joined = true;
   newPlayer.sessionId = sessionId;
      
   auto& joinedPlayer = newPlayer;
   
   JoinPlayerProcess(sessionRef, playerPawn);
   
   if (AllPlayerJoined()) {
      TryTransitToLoading();
   }
   
   return true;
}

bool Room::Leave(PlayerId playerId)
{
   SendBufferRef leaveResBuffer;
   SendBufferRef despawnBufferToOthers;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   {
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
   if (newSessionId == 0)
      return false;   // 유효하지 않은 sessionId
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;   // 방에 존재하지 않는 플레이어
   
   it->second.sessionId = newSessionId;
   return true;
}

void Room::JoinPlayerProcess(std::shared_ptr<PlayerSession>& session, PlayerPawn* playerPawn)
{
   SendBufferRef enterResBuffer;
   SendBufferRef entitiesSpawnBuffer;
   SendBufferRef playerInitBuffer;
   
   // 입장한 플레이어에게 방 스냅샷 전송하기
   {
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
         myEntityId->set_value(playerPawn->GetId().value);
      }
      enterResBuffer = ServerPacketHandler::MakeSendBuffer(res);
   }
   
   // 입장한 플레이어에게 월드의 Object 들의 스폰 정보 전송하기
   {
      se::room::N_EntitiesSpawn spawnPkt;
      {
         objectManager_.ForEachAlive([&](BaseObject* obj)
         {
            if (obj == nullptr)
               return;
            
            se::room::SpawnInfo tempInfo;
            if (!BuildSpawnInfo(obj, &tempInfo)) 
               return;
            
            spawnPkt.add_infos()->CopyFrom(tempInfo);
         });
      }
      entitiesSpawnBuffer = ServerPacketHandler::MakeSendBuffer(spawnPkt);
   }
   
   // 입장한 플레이어의 초기 값 세팅
   {
      se::game::N_PlayerInitSetup playerInitSetup;
      {
         auto health = playerPawn->GetHealth();
         int32 maxHp = health.GetMaxHp();
         int32 currentHp = health.GetHp();
         auto wallet = playerPawn->GetWallet();
         int32 money = wallet.GetBalance(CurrencyType::TimePoint);
         
         playerInitSetup.set_max_health(maxHp);
         playerInitSetup.set_current_health(currentHp);
         playerInitSetup.set_time_points(money);
         
         const auto& weapons = playerPawn->GetPlayerCombat()->GetWeaponSlots();
         for (const auto& weapon : weapons) {
            auto* weaponPtr = playerInitSetup.add_weapon_slots();
            weaponPtr->set_weapon_id(weapon.runtime.weaponId);
            auto* weaponStat = weaponPtr->mutable_stat();
            weaponStat->set_mag_capacity(weapon.stat.common.magCapacity);
            weaponStat->set_fire_interval(weapon.stat.common.fireIntervalSec);
            weaponStat->set_reload_time(weapon.stat.common.reloadTimeSec);

            switch (weapon.stat.common.category)
            {
            case WeaponCategory::Rifle:
               // None
               break;
               
            case WeaponCategory::Shotgun:
               {
                  if (std::holds_alternative<ShotgunStat>(weapon.stat.extra)) {
                     const ShotgunStat& shotgunStat = std::get<ShotgunStat>(weapon.stat.extra);
                     weaponStat->set_pellet_count(shotgunStat.pelletCount);
                     weaponStat->set_cone_angle(shotgunStat.coneAngleDegrees);
                  }
               }
               break;
               
            case WeaponCategory::Launcher:
               {
                  if (std::holds_alternative<LauncherStat>(weapon.stat.extra)) {
                     const LauncherStat& launcherStat = std::get<LauncherStat>(weapon.stat.extra);
                     weaponStat->set_projectile_speed(launcherStat.projectileSpeed);
                     weaponStat->set_explosion_radius(launcherStat.explosionRadius);
                  }
               }
               break;
            }
         }
         
      }
      playerInitBuffer = ServerPacketHandler::MakeSendBuffer(playerInitSetup);
   }
   
   if (enterResBuffer)
      session->Send(enterResBuffer);
   if (entitiesSpawnBuffer)
      session->Send(entitiesSpawnBuffer);
   if (playerInitBuffer)
      session->Send(playerInitBuffer);
}

bool Room::HandleLoadingComplete(PlayerId playerId)
{
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0) 
         return false;   // 유효하지 않은 playerId
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end()) {
         consoleLogger->Log(Color::Yellow, L"[Room] PlayerId %u not found in roomPlayers_ during loading complete handling\n", playerId);
         return false;
      }
      
      if (it->second.loaded) {
         consoleLogger->Log(Color::Yellow, L"[Room] Player is already loading complete\n", playerId);
         return true;   // 이미 로딩 완료 처리된 플레이어
      }
      
      it->second.loaded = true;
   }
   
   if (AllPlayerLoaded()) {
      Start();
   }
   
   return true;
}

bool Room::HandleMove(PlayerId playerId, const se::game::C_MoveReq& pkt)
{
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   // TODO: 기본적으로 본인 Player에겐 예외, 다만 유효성 판정 실패 시 보정 패킷을 보내야 한다
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      const auto& move = pkt.movement();
      const auto& velocity = move.velocity();
      const auto& pos = move.position();
      
      // TODO: 유효성 판정은 여기서
      //       bool 값으로 유효성 판정 결과를 받고, 유효하지 않은 경우 보정 패킷을 보내는 구조로 변경하기 (클라이언트와 서버의 위치가 달라지는 경우 보정 패킷을 보내는 구조로)
      
      playerPawn->SetPosition(Vector3{pos.x(), pos.y(), pos.z()});
      playerPawn->SetYaw(move.yaw());
      playerPawn->SetAimYaw(move.aim_yaw());
      playerPawn->SetPitch(move.pitch());
      playerPawn->SetVelocity(Vector3{velocity.x(), velocity.y(), 0.0f});
      playerPawn->SetMovementMode(move.movement_mode());
   }
   
   return true;
}

bool Room::HandleAim(PlayerId playerId, const se::game::C_AimReq& pkt)
{
   SendBufferRef aimBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
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
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      auto combatComp = playerPawn->GetCombatComponent();
      if (!combatComp) {
         consoleLogger->Log(Color::Yellow, L"[Room] PlayerPawn has no CombatComponent\n");
         return false;
      }
      
      AttackRequest attackReq;
      attackReq.weaponId = pkt.weapon_id();
      attackReq.shotSeed = pkt.shot_seed();
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
         
         noti.set_shot_seed(pkt.shot_seed());
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
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
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
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
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
   
   // TODO: 여기서 보낼 게 아니다 (재장전 완료 시간에 보내야 한다)
   if (reloadResultBuffer)
      sessionRef->Send(reloadResultBuffer);   // 재장전 결과를 해당 플레이어에게 전송
   
   if (reloadBroadcastBuffer)
      Broadcast(reloadBroadcastBuffer, playerId);   // 재장전한 플레이어를 제외한 나머지 플레이어들에게 재장전 정보 Broadcast
   
   return true;
}

bool Room::HandleWeaponChange(PlayerId playerId, const se::game::C_WeaponChangeReq& pkt)
{
   SendBufferRef weaponChangeBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
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

bool Room::HandleUseAbility(PlayerId playerId, const se::game::C_UseAbilityReq& pkt)
{
   SendBufferRef abilityUseBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      // TODO: Player Ability 사용 처리 로직 (예: Ability 효과 적용, 쿨타임 체크 등)
      //       유효 하다면 다음으로
      
      se::game::N_UseAbility noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         // TODO: 추가되는 정보가 있다면 noti에 더 넣어주기 (예: 타겟 정보, Direction 등)
         noti.set_ability_id(pkt.ability_id());
      }
      
      abilityUseBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (abilityUseBroadcastBuffer)
      Broadcast(abilityUseBroadcastBuffer);   // 모두에게 Ability 사용 정보 Broadcast
                                              // 본인에게도 보내는 이유는 버프형 스킬의 경우 이 패킷을 받은 뒤 부터 적용되도록
   
   return true;
}

bool Room::HandleUseItem(PlayerId playerId, const se::game::C_UseItemReq& pkt)
{
   SendBufferRef itemUseBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      // TODO: 아이템 사용 처리 로직 (예: 아이템 효과 적용, 인벤토리에서 아이템 제거 등)
      //       유효 하다면 다음으로
      //       그리고 여기서 만족하는 게 아니라 배그와 같이 사용하는 모션동안 사용이 취소될 수 있는 구조로 변경하기 (예: 회복 아이템 사용 중에 캔슬하면 아이템이 사용되지 않도록)
      
      se::game::N_UseItem noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         noti.set_item_id(pkt.item_id());
      }
      
      itemUseBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (itemUseBroadcastBuffer)
      Broadcast(itemUseBroadcastBuffer, playerId);   // 본인 제외 모두에게 아이템 사용 정보 Broadcast 
   
   return true;
}

bool Room::HandleChestInteract(PlayerId playerId, const se::game::C_ChestInteractReq& pkt)
{
   SendBufferRef chestInteractBroadcastBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      if (not playerPawn->IsHpAlive()) {
         // 사망한 Player가 Chest과 상호작용 시도 (정상적이지 않은 상황)
         return false;
      }
      
      const ObjectId chestId{pkt.chest_entity_id().value()};
      auto* chestObj = objectManager_.FindAs<ChestActor>(chestId);
      if (!chestObj) {
         consoleLogger->Log(Color::Yellow, L"[Room] ChestActor not found for chestId %u\n", chestId.value);
         return false;   // ChestActor가 존재하지 않음 (정상적이지 않은 상황)
      }
      
      int32 outError{0};
      if (chestObj->IsChest() and chestObj->CheckOpenPermission(*playerPawn, outError)) {
         if (outError != 0) {
            // 에러가 발생한 것
         }
         
         DropSpawnContext dropSpawnContext;
         dropSpawnContext.reason = DropReason::Chest;
         dropSpawnContext.owner = chestId;
         dropSpawnContext.instigator = playerPawn->GetId();
         dropSpawnContext.lootBundle = chestObj->GenerateDrops();
         if (not dropSpawnContext.lootBundle.Empty()) {
            auto& dropSystem = roomGameSystem_.GetDropSystem();
            DropSpawnResult result = dropSystem.DropItems(dropSpawnContext);
            if (not result.spawned) {
               // 드랍 처리 실패 (정상적이지 않은 상황)
               consoleLogger->Log(Color::Yellow, L"[Room] Failed to drop items from chestId %u\n", chestId.value);
               return false;
            }
         }
         
         se::game::N_ChestInteracted noti;
         {
            auto* entityIdPtr = noti.mutable_entity_id();
            entityIdPtr->set_value(it->second.pawnObjectId.value);
            auto* chestIdPtr = noti.mutable_chest_entity_id();
            chestIdPtr->set_value(chestId.value);
         }
         chestInteractBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
      }
   }
   
   if (chestInteractBroadcastBuffer)
      Broadcast(chestInteractBroadcastBuffer, playerId);   // Chest과 상호작용한
   
   return true;
}

bool Room::HandlePickupItem(PlayerId playerId, const se::game::C_PickupItemReq& pkt)
{
   SendBufferRef pickupBroadcastBuffer;
   SendBufferRef itemDespawnBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      const ObjectId& pawnId = it->second.pawnObjectId;
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(pawnId);
      if (!playerPawn)
         return false;
      
      ObjectId itemObjectId{pkt.item_entity_id().value()};
      auto* itemObj = objectManager_.Find(itemObjectId);
      if (!itemObj)
         return false;
      
      bool itemActorRemove = objectManager_.RequestDestroy(itemObjectId);
      
      auto* item = dynamic_cast<WorldItemActor*>(itemObj);
      if (!item)
         return false;
      
      const auto& itemStack = item->GetItemStack();
      if (!itemStack.IsValid())
         return false;
      
      playerPawn->AddItem(itemStack.id, itemStack.count, ItemChangeContext(ItemChangeReason::Loot));
      
      se::game::N_PickupItem pickupItemNoti;
      {
         auto* entityPtr = pickupItemNoti.mutable_entity_id();
         entityPtr->set_value(pawnId.value);
         
         auto* itemIdPtr = pickupItemNoti.mutable_item_entity_id();
         itemIdPtr->set_value(itemObjectId.value);
      }
      pickupBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(pickupItemNoti);
      
      se::room::N_EntityDespawn itemDespawnNoti;
      {
         auto* entityIdPtr = itemDespawnNoti.mutable_entity_id();
         entityIdPtr->set_value(itemObjectId.value);
      }
      itemDespawnBuffer = ServerPacketHandler::MakeSendBuffer(itemDespawnNoti);
   }
   
   if (pickupBroadcastBuffer)
      Broadcast(pickupBroadcastBuffer);   // 아이템을 획득 (이펙트를 위해)
   
   // TODO: 아이템이 사라지는 패킷은 아이템 획득 이펙트가 재생된 후에 잠시 딜레이를 두고 보내는 구조로 변경하기 (클라이언트에서 아이템 획득 이펙트가 재생된 후에 아이템이 사라지는 구조로)
   if (itemDespawnBuffer)
      Broadcast(itemDespawnBuffer);   // 아이템이 사라졌음을 모두에게 Broadcast
   
   return true;
}

bool Room::HandleUseStore(PlayerId playerId, const se::game::C_UseStoreReq& pkt)
{
   SendBufferRef useStoreResultBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      StoreBuyRequest request;
      request.playerId = playerPawn->GetId();
      request.storeId = ObjectId{pkt.store_entity_id().value()};
      request.entryId = pkt.store_item_id();
      StoreBuyResult result = roomGameSystem_.GetStoreSystem().Buy(request);
      
      se::game::S_UseStoreRes res;
      {
         res.set_success(result.success);
         
         if (not result.success) {
            auto* resultPtr = res.mutable_result();
            resultPtr->set_message("Failed to purchase item.");
            resultPtr->set_code(se::common::ERR_INSUFFICIENT_TIME_POINTS);        // TODO: 구매 실패 사유 (예: 아이템 부족, 인벤토리 공간 부족 등) 올바르게 적기
                                                                                       //       Protocol Enum 확장 필요
         }
      }
      
      useStoreResultBuffer = ServerPacketHandler::MakeSendBuffer(res);
   }
   
   if (useStoreResultBuffer)
      sessionRef->Send(useStoreResultBuffer);   // 상점 이용 결과를 해당 플레이어에게 전송
   
   return true;
}

bool Room::HandleSetSavePoint(PlayerId playerId, const se::game::C_SetSavePointReq& pkt)
{
   // THINK: 안전상 쿨타임이 존재해야 하지만 우선은 쿨타임 없이 바로 적용하는 구조로 (현재는 패킷이 너무 자주 요청 될 수 있음...)
   
   SendBufferRef setSavePointResultBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      const auto& savePointPos = pkt.position();
      const Vector3 savePos{ savePointPos.x(), savePointPos.y(), savePointPos.z() };
      
      bool saved = playerPawn->TrySetSavePoint(savePos);
      
      se::game::S_SetSavePointRes res;
      {
         res.set_success(saved);   // TEMP: 세이브 포인트 설정 성공 여부 (실제 로직에서는 유효성 판정 결과에 따라 결정되어야 함)
         
         if (saved) {
            auto* savePosPtr = res.mutable_position();
            savePosPtr->set_x(savePointPos.x());
            savePosPtr->set_y(savePointPos.y());
            savePosPtr->set_z(savePointPos.z());
         }
         else {
            auto* resultPtr = res.mutable_result();
            resultPtr->set_message("Failed to set save point.");
            resultPtr->set_code(se::common::ERR_ABILITY_NOT_AVAILABLE);
         }
      }
      setSavePointResultBuffer = ServerPacketHandler::MakeSendBuffer(res);
   }
   
   if (setSavePointResultBuffer)
      sessionRef->Send(setSavePointResultBuffer);   // 세이브 포인트 설정 결과를 해당 플레이어에게 전송
   
   return true;
}

bool Room::HandleJump(PlayerId playerId, const se::game::C_JumpReq& pkt)
{
   SendBufferRef jumpBroadcastBuffer;
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      playerPawn->SetJumping(true);
      
      se::game::N_Jump noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
      }
      jumpBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (jumpBroadcastBuffer)
      Broadcast(jumpBroadcastBuffer, playerId);   // 점프한 플레이어를 제외한 나머지 플레이어들에게 점프 정보 Broadcast
   
   return true;
}

bool Room::HandleJumpLand(PlayerId playerId, const se::game::C_JumpLand& pkt)
{
   SendBufferRef landBroadcastBuffer;
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      playerPawn->SetJumping(false);
      playerPawn->SetDoubleJumping(false);
      
      se::game::N_JumpLand noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
      }
      landBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (landBroadcastBuffer)
      Broadcast(landBroadcastBuffer, playerId);   // 착지한 플레이어를 제외한 나머지 플레이어들에게 착지 정보 Broadcast
   
   return true;
}

bool Room::HandleDoubleJump(PlayerId playerId, const se::game::C_DoubleJumpReq& pkt)
{
   SendBufferRef doubleJumpBroadcastBuffer;
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      playerPawn->SetJumping(false);
      playerPawn->SetDoubleJumping(true);
      
      se::game::N_DoubleJump noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
      }
      
      doubleJumpBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (doubleJumpBroadcastBuffer)
      Broadcast(doubleJumpBroadcastBuffer, playerId);   // 더블 점프한 플레이어를 제외한 나머지 플레이어들에게 더블 점프 정보 Broadcast

   return true;
}

bool Room::HandleCrouch(PlayerId playerId, const se::game::C_CrouchReq& pkt)
{
   SendBufferRef crouchBroadcastBuffer;
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      playerPawn->SetCrouching(pkt.is_crouching());
      
      se::game::N_Crouch crouchNoti;
      {
         auto* entityIdPtr = crouchNoti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         crouchNoti.set_is_crouching(playerPawn->IsCrouching());
      }
      
      crouchBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(crouchNoti);
   }
   
   if (crouchBroadcastBuffer)
      Broadcast(crouchBroadcastBuffer, playerId);   // 크로치 상태를 변경한 플레이어를 제외한 나머지 플레이어들에게 크로치 상태 변경 정보 Broadcast
   
   return true;
}

bool Room::HandleWireAction(PlayerId playerId, const se::game::C_WireActionReq& pkt)
{
   SendBufferRef wireBroadcastBuffer;
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      playerPawn->SetWired(true);
      
      se::game::N_WireAction noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         auto* anchorPosPtr = noti.mutable_anchor_point();
         anchorPosPtr->CopyFrom(pkt.anchor_point());
      }
      wireBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (wireBroadcastBuffer)
      Broadcast(wireBroadcastBuffer, playerId);   // 와이어 액션을 시작한 플레이어를 제외한 나머지 플레이어들에게 와이어 액션 정보 Broadcast
   
   return true;
}

bool Room::HandleWireActionEnd(PlayerId playerId, const se::game::C_WireActionEnd& pkt)
{
   SendBufferRef wireEndBroadcastBuffer;
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      playerPawn->SetWired(false);
      
      se::game::N_WireActionEnd noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
      }
      wireEndBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (wireEndBroadcastBuffer)
      Broadcast(wireEndBroadcastBuffer, playerId);   // 와이어 액션을 종료한 플레이어를 제외한 나머지 플레이어들에게 와이어 액션 종료 정보 Broadcast
   
   return true;
}

bool Room::HandleWireLaunch(PlayerId playerId, const se::game::C_WireLaunchReq& pkt)
{
   SendBufferRef wireLaunchBuffer;
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      se::game::N_WireLaunch noti;
      {
         auto* entityIdPtr = noti.mutable_entity_id();
         entityIdPtr->set_value(it->second.pawnObjectId.value);
         
         auto* launchStartPtr = noti.mutable_start_position();
         launchStartPtr->set_x(pkt.start_position().x());
         launchStartPtr->set_y(pkt.start_position().y());
         launchStartPtr->set_z(pkt.start_position().z());
         
         auto* launchDirPtr = noti.mutable_direction();
         launchDirPtr->set_x(pkt.direction().x());
         launchDirPtr->set_y(pkt.direction().y());
         launchDirPtr->set_z(pkt.direction().z());
      }
      
      wireLaunchBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   }
   
   if (wireLaunchBuffer)
      Broadcast(wireLaunchBuffer, playerId);   // 와이어 런치를 시작한 플레이어를 제외한 나머지 플레이어들에게 와이어 런치 정보 Broadcast
   
   return true;
}

bool Room::HandleEquipItem(PlayerId playerId, const se::game::C_EquipItemReq& pkt)
{
   SendBufferRef equipItemResultBuffer;
   SendBufferRef equipItemBroadcastBuffer;
   
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      const uint32 itemId = pkt.item_id();
      
      auto& inventory = playerPawn->GetInventory();
      bool hasItem = inventory.HasItem(itemId, 1);
      
      if (not hasItem) {
         // 인벤토리에 아이템이 없는 경우 (정상적이지 않은 상황)
         consoleLogger->Log(Color::Yellow, L"[Room] Equip Item Failed: Player does not have itemId %u in inventory\n", itemId);
      }
      else {
         se::game::N_EquipItem noti;
         {
            auto* entityIdPtr = noti.mutable_entity_id();
            entityIdPtr->set_value(it->second.pawnObjectId.value);
         
            noti.set_item_id(itemId);
         }
         equipItemBroadcastBuffer = ServerPacketHandler::MakeSendBuffer(noti);
      }
      
      se::game::S_EquipItemRes res;
      {
         res.set_success(hasItem);
         if (not hasItem) {
            auto* resultPtr = res.mutable_result();
            resultPtr->set_message("Player does not have the item in inventory.");
            resultPtr->set_code(se::common::ERR_ITEM_NOT_FOUND);
         }
         res.set_item_id(itemId);
      }
      equipItemResultBuffer = ServerPacketHandler::MakeSendBuffer(res);
   }
   
   if (equipItemResultBuffer)
      sessionRef->Send(equipItemResultBuffer);   // 아이템 장착 결과를 해당 플레이어에게 전송
   
   if (equipItemBroadcastBuffer)
      Broadcast(equipItemBroadcastBuffer, playerId);   // 아이템을 장착한
   
   return true;
}

bool Room::HandleSkillEquip(PlayerId playerId, const se::game::C_SkillEquipReq& pkt)
{
   SendBufferRef skillEquipResultBuffer;
   
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;   // 방에 존재하지 않는 플레이어
      
      if (not it->second.loaded)
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      const uint32 skillId = pkt.skill_id();
      const uint32 slotIndex = pkt.slot_index();
      
      auto& skill = playerPawn->GetSkill();
      bool equip = skill.EquipSkill(skillId, static_cast<uint8>(slotIndex));
      
      se::game::S_SkillEquipRes res;
      {
         res.set_success(equip);
         if (not equip) {
            auto* resultPtr = res.mutable_result();
            consoleLogger->Log(Color::Yellow, L"[Room] Failed to equip skillId %u to slotIndex %u for playerId %u\n", skillId, slotIndex, playerId);
            resultPtr->set_message("Failed to equip skill.");
            resultPtr->set_code(se::common::ERR_UNKNOWN);   // TODO: 실패 사유에 따른 코드 구체화 필요
         }
         else {
            res.set_skill_id(skillId);
            res.set_slot_index(slotIndex);
         }
      }
      skillEquipResultBuffer = ServerPacketHandler::MakeSendBuffer(res);
   }
   
   if (skillEquipResultBuffer)
      sessionRef->Send(skillEquipResultBuffer);   // 스킬 장착 결과를 해당 플레이어에게 전송
   
   return false;
}

// TODO: Monster 우선 만들고 아래 핸들러 구현하기 (Monster Spawn, Monster Attack 등)
bool Room::HandleSpawnMonster(PlayerId playerId, const se::test::C_SpawnMonsterReq& pkt)
{
   return false;
}

bool Room::HandleSpawnChest(PlayerId playerId, const se::test::C_SpawnChestReq& pkt)
{
   const auto& pos = pkt.spawn_position();
   return SpawnChest(Vector3{pos.x(), pos.y(), pos.z()}, 1);    // TEMP: Table ID는 클라이언트에서 모른다
}

bool Room::HandleSpawnStore(PlayerId playerId, const se::test::C_SpawnStoreReq& pkt)
{
   const auto& pos = pkt.spawn_position();
   return SpawnStore(Vector3{pos.x(), pos.y(), pos.z()});
}

bool Room::HandleItem(PlayerId playerId, const se::test::C_ItemReq& pkt)
{
   return GiveItem(playerId, ItemStack{pkt.item_id(), static_cast<int32>(pkt.quantity())});
}

bool Room::HandleMoney(PlayerId playerId, const se::test::C_MoneyReq& pkt)
{
   return GiveMoney(playerId, static_cast<int32>(pkt.amount()));
}

bool Room::HandleHealth(PlayerId playerId, const se::test::C_HealthReq& pkt)
{
   if (playerId == 0)
      return false;
      
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;   // 방에 존재하지 않는 플레이어
      
   auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
   if (!playerPawn)
      return false;
   
   auto& health = playerPawn->GetHealth();
   int32 currentHp = health.GetHp();
   health.SetHpUnsafe(static_cast<int32>(pkt.health()));
   int32 newHp = health.GetHp();
   NotifyHealthChange(playerId, newHp, newHp - currentHp);
   return true;
}

bool Room::HandleMaxHealth(PlayerId playerId, const se::test::C_MaxHealthReq& pkt)
{
   if (playerId == 0)
      return false;
      
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;   // 방에 존재하지 않는 플레이어
      
   auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
   if (!playerPawn)
      return false;
   
   auto& health = playerPawn->GetHealth();
   health.SetHpUnsafe(static_cast<int32>(pkt.max_health()));
   int32 newMapHp = health.GetMaxHp();
   int32 currentHp = health.GetMaxHp();
   NotifyMaxHealthChange(playerId, newMapHp, currentHp);
   return true;
}

bool Room::HandleZoneStop(PlayerId playerId, const se::test::C_ZoneStopReq& pkt)
{
   ZoneSystem& zoneSystem = roomGameSystem_.GetZoneSystem();
   zoneSystem.SetProgressing(false);
   
   NotifyZoneFlow(false);
   
   return true;
}

bool Room::HandleZoneStart(PlayerId playerId, const se::test::C_ZoneStartReq& pkt)
{
   ZoneSystem& zoneSystem = roomGameSystem_.GetZoneSystem();
   zoneSystem.SetProgressing(true);
   
   NotifyZoneFlow(true);
   
   return true;
}

bool Room::HandleZoneReset(PlayerId playerId, const se::test::C_ZoneResetReq& pkt)
{
   ZoneSystem& zoneSystem = roomGameSystem_.GetZoneSystem();
   zoneSystem.ReStart();
   
   return true;
}

bool Room::HandleZoneDamageOff(PlayerId playerId, const se::test::C_ZoneDamageOffReq& pkt)
{
   ZoneSystem& zoneSystem = roomGameSystem_.GetZoneSystem();
   zoneSystem.SetDamageApplied(false);
   
   return true;
}

bool Room::HandleZoneDamageOn(PlayerId playerId, const se::test::C_ZoneDamageOnReq& pkt)
{
   ZoneSystem& zoneSystem = roomGameSystem_.GetZoneSystem();
   zoneSystem.SetDamageApplied(true);
   
   return true;
}

WorldItemActor* Room::SpawnItem(const SpawnWorldItemParams& params)
{
   WorldItemActor* item = SpawnObject<WorldItemActor>(ObjectFlags::None);
   if (!item) {
      return nullptr;
   }
   
   item->SetPosition(params.position);
   ReplicationSpawn(item, params.itemStack.id, item->GetYaw(), params.itemStack.count);
   return item;
}

bool Room::SpawnChest(const Vector3& pos, int32 tableId)
{
   auto* chest = SpawnObject<ChestActor>(ObjectFlags::None);
   if (!chest)
      return false;
   
   chest->SetPosition(pos);
   chest->SetTableId(tableId);
   
   ReplicationSpawn(chest, /*templateId=*/0, chest->GetYaw());    // TODO: templateId는 나중에 생각하기
   
   return true;
}

bool Room::SpawnStore(const Vector3& pos)
{
   auto* store = SpawnObject<StoreActor>(ObjectFlags::None);
   if (!store)
      return false;
   
   store->SetPosition(pos);
   
   ReplicationSpawn(store, /*templateId=*/0, store->GetYaw());    // TODO: templateId는 나중에 생각하기
   
   return true;
}

bool Room::Start()
{
   if (roomState_ != RoomState::Loading) 
      return false;
   
   if (!roomGameSystem_.Start())
      return false;
   
   roomState_ = RoomState::Playing;
   if (ownerShard_)
      ownerShard_->ScheduleRoomFirstTick(roomId_);
   
   BroadcastGameStart();
   return true;
}

void Room::UpdateTick(const RepFrame& frame)
{
   // Room 정책
   // GameSystem 진행
   // Object Tick 진행
   const float deltaSeconds = frame.dt.count() / 1000.0f;
   
   roomGameSystem_.Update(deltaSeconds);

   objectManager_.ForEachTickableAlive([deltaSeconds](BaseObject* obj)
   {
      obj->__Tick(deltaSeconds);
   });
   
   // objectManager_.SweepDestroy();   // 오브젝트 제거 처리

   roomGameSystem_.GetReplicationSystem().FlushImmediate(frame);
   roomGameSystem_.GetReplicationSystem().FlushPeriodic(frame);
}

bool Room::TraceHit(const SE::Physics::Ray& ray, ObjectId exceptId, SE::Physics::Hit::HitResult& outHit) const
{
   bool hasHit = false;
   float closestT = std::numeric_limits<float>::max();
   
   outHit.Reset();
   
   objectManager_.ForEachAlive([&](BaseObject* obj)
   {
      if (!obj)
         return;
      
      if (obj->GetId() == exceptId)
         return;   // 제외할 오브젝트는 건너뛰기
      
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

TimerId Room::ScheduleAt(TimePoint executeAt, Job job)
{
   return ownerShard_->ScheduleAt(executeAt, std::move(job));
}

TimerId Room::ScheduleAfter(Duration delay, Job job)
{
   return ownerShard_->ScheduleAfter(delay, std::move(job));
}

bool Room::CancelScheduled(TimerId timerId)
{
   return ownerShard_->CancelTimer(timerId);
}

bool Room::HasPlayer(PlayerId playerId) const
{
   return roomPlayers_.contains(playerId);
}

SessionId Room::GetSessionId(PlayerId playerId) const
{
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return 0;   // 방에 존재하지 않는 플레이어
   
   return it->second.sessionId;
}

ObjectId Room::GetObjectId(PlayerId playerId) const
{
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return ObjectId{};   // 방에 존재하지 않는 플레이어
   
   return it->second.pawnObjectId;
}

bool Room::LaunchRocket(const Vector3& pos, const Vector3& dir, Pawn* ownerPawn, int32 damage, float speed, uint32 lifetimeMs, float radius)
{
   if (!ownerPawn)
      return false;   // 유효하지 않은 발사체 소유자 Pawn
   
   ProjectileActor* rocket = SpawnObject<ProjectileActor>(ObjectFlags::Replicable | ObjectFlags::Tickable);
   if (!rocket)
      return false;  // 발사체 생성 실패
   
   rocket->Init(ownerPawn->GetId(), pos, dir * speed, damage, lifetimeMs, radius);
   ReplicationSpawn(rocket, /*templateId=*/0, rocket->GetYaw());  // TODO: templateId는 나중에 생각하기
   return true;
}

PlayerPawn* Room::CreatePreparedPlayerPawn(PlayerId playerId, const Vector3& spawnPos)
{
   auto playerPawn = SpawnObject<PlayerPawn>(ObjectFlags::Replicable | ObjectFlags::Tickable);
   if (!playerPawn) {
      return nullptr;
   }
   
   playerPawn->SetPosition(spawnPos);
   playerPawn->SetSavedRespawnPosition(spawnPos);
   playerPawn->SetOwnerPlayerId(playerId);
   return playerPawn;
}

bool Room::GiveItem(PlayerId playerId, const ItemStack& itemStack)
{
   if (playerId == 0)
      return false;
      
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;   // 방에 존재하지 않는 플레이어
      
   auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
   if (!playerPawn)
      return false;
   
   InventoryOpResult result = playerPawn->AddItem(itemStack.id, itemStack.count, ItemChangeContext{ItemChangeReason::System});
   return result.accepted;
}

bool Room::GiveMoney(PlayerId playerId, int32 amount)
{
   if (playerId == 0)
      return false;
      
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;   // 방에 존재하지 않는 플레이어
      
   auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
   if (!playerPawn)
      return false;
   
   MoneyChangeResult result = playerPawn->AddMoney(CurrencyType::TimePoint, amount, MoneyChangeContext{MoneyChangeReason::System});
   return result.accepted;
}

void Room::Broadcast(std::shared_ptr<SendBuffer> sendBuffer, PlayerId exceptPlayerId)
{
   if (not sendBuffer)
      return;   // 유효하지 않은 SendBuffer

   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (playerId == exceptPlayerId)
         continue;   // 제외할 플레이어는 건너뛰기
      
      if (roomPlayer.sessionId == 0)
         continue;   // 유효하지 않은 세션 ID인 플레이어는 건너뛰기
      
      if (auto session = sessionManager_.FindBySessionId(roomPlayer.sessionId)) {
         session->Send(sendBuffer);
      }
   }
}

bool Room::SendToPlayer(PlayerId playerId, SendBufferRef buffer)
{
   if (playerId == 0 or buffer == nullptr)
      return false;  // 유효하지 않은 playerId 또는 SendBuffer
   
   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;  // 방에 존재하지 않는 플레이어
   
   auto session = sessionManager_.FindByPlayerId(playerId);
   if (!session)
      return false;  // 플레이어의 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   session->Send(buffer);
   return true;
}

void Room::BroadcastReplication(SendBufferRef sendBuffer, PlayerId exceptPlayerId)
{
   Broadcast(std::move(sendBuffer), exceptPlayerId);
}

void Room::SendReplication(PlayerId playerId, SendBufferRef sendBuffer)
{
   SendToPlayer(playerId, std::move(sendBuffer));
}

void Room::BroadcastGameStart()
{
   se::game::N_GameStart noti;
   
   SendBufferRef gameStartBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   if (gameStartBuffer)
      Broadcast(gameStartBuffer);   // 모두에게 게임 시작 정보 Broadcast
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

void Room::BroadcastRespawn(ObjectId objectId)
{
   Pawn* pawn = objectManager_.FindAs<Pawn>(objectId);
   if (pawn == nullptr)
      return;   // 유효하지 않은 Pawn 객체
   
   const Vector3& respawnPos = pawn->GetSavedRespawnPosition();
   const float respawnYaw = pawn->GetYaw();
   
   se::game::N_EntityRespawned noti;
   {
      auto* entityIdPtr = noti.mutable_entity_id();
      entityIdPtr->set_value(objectId.value);
      
      auto* transformPtr = noti.mutable_transform();
      auto* positionPtr = transformPtr->mutable_position();
      positionPtr->set_x(respawnPos.x);
      positionPtr->set_y(respawnPos.y);
      positionPtr->set_z(respawnPos.z);
      transformPtr->set_yaw(respawnYaw);
   }
   
   SendBufferRef respawnBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   if (respawnBuffer)
      Broadcast(respawnBuffer);     // 모두에게 리스폰 정보 Broadcast
                                    // Local Control Player의 경우는 Set Position 하도록 (Set Yaw 까지도 가능)
}

void Room::NotifyItemChange(PlayerId playerId, uint32 itemId, int32 newCount, int32 deltaCount)
{
   RepEvent itemChangeEvent;
   ReplicateEventSet(itemChangeEvent, RepEventType::ItemChange);
   itemChangeEvent.header.playerId = playerId;
   itemChangeEvent.header.source = roomPlayers_[playerId].pawnObjectId;
   itemChangeEvent.payload = ItemChangeEvent{itemId, newCount, deltaCount};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(itemChangeEvent);
}

void Room::NotifyHealthChange(PlayerId id, int newHealth, int deltaHealth)
{
   RepEvent healthChangeEvent;
   ReplicateEventSet(healthChangeEvent, RepEventType::HealthChange);
   healthChangeEvent.header.playerId = id;
   healthChangeEvent.header.source = roomPlayers_[id].pawnObjectId;
   healthChangeEvent.payload = HealthChangeEvent{newHealth, deltaHealth};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(healthChangeEvent);
}

void Room::NotifyMaxHealthChange(PlayerId id, int newMaxHealth, int newHealth)
{
   RepEvent maxHealthChangeEvent;
   ReplicateEventSet(maxHealthChangeEvent, RepEventType::MaxHealthChange);
   maxHealthChangeEvent.header.playerId = id;
   maxHealthChangeEvent.header.source = roomPlayers_[id].pawnObjectId;
   maxHealthChangeEvent.payload = MaxHealthChangeEvent{newMaxHealth, newHealth};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(maxHealthChangeEvent);
}

void Room::NotifyTimePointChange(PlayerId id, int newTimePoint, int deltaTimePoint)
{
   RepEvent moneyChangeEvent;
   ReplicateEventSet(moneyChangeEvent, RepEventType::MoneyChange);      // Resource
   moneyChangeEvent.header.playerId = id;
   moneyChangeEvent.payload = MoneyChangeEvent{newTimePoint, deltaTimePoint};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(moneyChangeEvent);
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

void Room::NotifyZoneFlow(bool flowing)
{
   RepEvent zoneFlowEvent;
   ReplicateEventSet(zoneFlowEvent, RepEventType::ZoneFlow);
   zoneFlowEvent.payload = ZoneFlowEvent{flowing};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(zoneFlowEvent);
}

void Room::ReplicateEventSet(RepEvent& ev, RepEventType eventType)
{
   ev.header.type = eventType;
   ev.header.timeMs = std::chrono::duration_cast<Milliseconds>(Clock::now().time_since_epoch()).count();
   ev.header.tick = tickSeq_;
}

void Room::ReplicationSpawn(Actor* actor, uint32 templateId, float yaw, uint32 amount)
{
   RepEvent spawnEvent;
   ReplicateEventSet(spawnEvent, RepEventType::Spawn);
   spawnEvent.header.source = actor->GetId();
   spawnEvent.payload = SpawnEvent{actor->GetObjectType(), templateId, actor->GetPosition(), actor->GetVelocity(), yaw, amount};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(spawnEvent);
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
   
   if (KillerIsPlayer and victim->IsMonster()) {
      // NPC Kill 처리...
      return;
   }
   
   if (attacker->IsMonster() and VictimIsPlayer) {
      // Player가 NPC에게 죽은 경우 처리...
      return;
   }
}

void Room::HandlePawnDeath(ObjectId pawnId, const DamageResult& damageResult)
{
   roomGameSystem_.OnPawnDeath(pawnId);
   BroadcastDeath(pawnId);
}

void Room::HandlePawnRespawn(ObjectId pawnId)
{
   BroadcastRespawn(pawnId);
}

void Room::OnZoneChanged(uint32 phase, const ZoneCircle& newZone, float waitDuration, float shrinkDuration)
{
   se::game::N_TimeStormChange noti;
   {
      auto* centerPtr = noti.mutable_center();
      centerPtr->set_x(newZone.center.x);
      centerPtr->set_y(newZone.center.y);
      centerPtr->set_z(newZone.center.z);
      
      noti.set_radius(newZone.radius);
      noti.set_waiting_time(waitDuration);
      noti.set_shrinking_time(shrinkDuration);
   }
   
   SendBufferRef zoneChangeBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   if (zoneChangeBuffer) {
      Broadcast(zoneChangeBuffer);
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

bool Room::AllPlayerJoined() const
{
   if (roomPlayers_.empty())
      return false;   // 플레이어가 한 명도 없는 경우
   
   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (!roomPlayer.joined) {
         return false;   // 한 명이라도 아직 입장하지 않은 플레이어가 있는 경우
      }
   }
   
   return true;   // 모든 플레이어가 입장한 경우
}

bool Room::AllPlayerLoaded() const
{
   if (roomPlayers_.empty())
      return false;   // 플레이어가 한 명도 없는 경우
   
   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (!roomPlayer.loaded) {
         return false;   // 한 명이라도 아직 로딩하지 않은 플레이어가 있는 경우
      }
   }
   
   return true;   // 모든 플레이어가 로딩한 경우
}

void Room::TryTransitToLoading()
{
   if (roomState_ != RoomState::WaitingForPlayers)
      return;

   if (!AllPlayerJoined())
      return;

   roomState_ = RoomState::Loading;
}

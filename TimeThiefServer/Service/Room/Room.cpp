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
#include "Content/Object/Actor/SubProjectile/GrenadeActor.h"
#include "Data/GameDataManager.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Ray/RaycastHit.h"

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
         return BuildStoreSpawnInfo(static_cast<StoreActor*>(obj), outInfo);
         
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
      if (!monsterPawn or !outInfo)
         return false;
      
      FillSpawnInfoBase(monsterPawn, monsterPawn->GetTemplateId(), outInfo);
      
      auto* detailPtr = outInfo->mutable_monster_info();
      auto* movementPtr = detailPtr->mutable_movement();
      
      auto* posPtr = movementPtr->mutable_position();
      const SE::Math::Vector3& pos = monsterPawn->GetPosition();
      posPtr->set_x(pos.x);
      posPtr->set_y(pos.y);
      posPtr->set_z(pos.z);
      
      movementPtr->set_yaw(monsterPawn->GetYaw());
      movementPtr->set_pitch(0.0f);    // monster가 pitch 개념이 있는지는 모르겠지만 일단 0으로 고정

      return true;
   }
   
   bool BuildItemSpawnInfo(WorldItemActor* item, se::room::SpawnInfo* outInfo)
   {
      if (!item or !outInfo)
         return false;
      
      FillSpawnInfoBase(item, item->GetItemStack().id, outInfo);
      
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
}

void Room::PostCreate()
{
   objectManager_.SetRoom(shared_from_this());
}

void Room::Close()
{
   if (closeTimerId_ != 0) {
      CancelScheduled(closeTimerId_);
      closeTimerId_ = 0;
   }
   
   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (not roomPlayer.joined) 
         continue;
      
      ownerShard_->RoomPlayerLeave(playerId);
   }
   
   BroadcastRoomClose();
}

bool Room::Init(GameShard* ownerShard, const GameDataManager& gameDataManager, const GameConfig& gameConfig)
{
   ownerShard_ = ownerShard;
   
   if (!roomGameSystem_.Init(this, gameDataManager, gameConfig))
      return false;
   
   gameDataManager_ = &gameDataManager;
   
   return true;
}

void Room::SetPlayer(const std::vector<PlayerId>& playerIds, const std::vector<std::string>& playerNames)
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

      RoomPlayer roomPlayer{};
      roomPlayer.playerId = playerId;
      roomPlayer.nickname = playerNames[i];

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
   const MapPlacementData& placementData = gameDataManager_->GetMapPlacementData();

   for (const StorePlacement& store : placementData.interactions.stores) {
      const PlacementTransform& transform = store.transform;
      CreateStoreActor(transform.position, transform.yaw);
   }

   for (const ChestPlacement& chest : placementData.interactions.chests) {
      const PlacementTransform& transform = chest.transform;
      CreateChestActor(transform.position, chest.lootTableId, transform.yaw);
   }

   for (const MonsterSpawnGroupPlacement& monsterGroup : placementData.monsters.spawnGroups) {
      if (monsterGroup.isBoss)
         continue;

      const size_t candidateCount = monsterGroup.spawnCandidates.size();
      if (candidateCount == 0)
         continue;

      const size_t spawnCount = std::min<size_t>(monsterGroup.spawnCount, candidateCount);
      std::vector<size_t> shuffledIndices;
      shuffledIndices.reserve(candidateCount);
      for (size_t i = 0; i < candidateCount; ++i) {
         shuffledIndices.push_back(i);
      }

      for (size_t i = candidateCount; i > 1; --i) {
         const size_t j = static_cast<size_t>(rng_.NextU32(static_cast<uint32>(i)));
         std::swap(shuffledIndices[i - 1], shuffledIndices[j]);
      }

      for (size_t i = 0; i < spawnCount; ++i) {
         const PlacementTransform& transform = monsterGroup.spawnCandidates[shuffledIndices[i]];
         CreateMonsterActor(transform.position, monsterGroup.templateId, transform.yaw);
      }
   }
}

bool Room::Join(PlayerId playerId, SessionId sessionId)
{
   if (playerId == 0 or sessionId == 0)      // 유효하지 않은 playerId 또는 sessionId
      return false;
   
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindBySessionId(sessionId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   if (sessionRef->GetState() != PlayerSessionState::MatchingSucc)
      return false;     // 세션이 매칭 성공 상태가 아님 (정상적이지 않은 상황)
   
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
   
   sessionRef->SetState(PlayerSessionState::InRoom);
      
   JoinPlayerProcess(playerId, playerPawn);
   
   if (AllPlayerJoined()) {
      TryTransitToLoading();
   }
   
   return true;
}

bool Room::Leave(PlayerId playerId)
{
   SendBufferRef leaveResBuffer;
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef)
      return false;
   
   if (playerId == 0)
      return false;   // 유효하지 않은 playerId

   auto it = roomPlayers_.find(playerId);
   if (it == roomPlayers_.end())
      return false;   // 방에 존재하지 않는 플레이어
   
   if (sessionRef->GetState() != PlayerSessionState::InRoom and sessionRef->GetState() != PlayerSessionState::Closing)
      return false;  // 세션이 방 상태가 아님 (정상적이지 않은 상황)
      
   {
      se::room::S_RoomLeaveRes res;
      res.set_success(true);
      
      leaveResBuffer = ServerPacketHandler::MakeSendBuffer(res);
   }
   
   const ObjectId pawnId = it->second.pawnObjectId;
   
   if (pawnId != ObjectId{}) {
      HandleDespawn(pawnId);
   }
   
   if (sessionRef->GetState() == PlayerSessionState::InRoom)
      sessionRef->SetState(PlayerSessionState::InLobby);
   
   ownerShard_->RoomPlayerLeave(playerId);   // 샤드에 플레이어 퇴장 알리기
   
   if (leaveResBuffer)
      SendToPlayer(playerId, leaveResBuffer);   // 퇴장한 플레이어에게 퇴장 결과 전송
   
   roomPlayers_.erase(it);
   CheckGameEndCondition();      // 플레이어 퇴장으로 인해 게임 종료 조건이 충족되는지 확인
   CheckRoomCloseCondition();    // 플레이어 퇴장으로 인해 방 종료 조건이 충족되는지 확인 (예: 모든 플레이어 퇴장)
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

void Room::JoinPlayerProcess(PlayerId playerId, PlayerPawn* playerPawn)
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
            roomPlayer->set_nickname(exPlayer.nickname);
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
         int32 moveSpeed = playerPawn->GetSpeed();
         
         playerInitSetup.set_max_health(maxHp);
         playerInitSetup.set_current_health(currentHp);
         playerInitSetup.set_time_points(money);
         playerInitSetup.set_move_speed(static_cast<float>(moveSpeed));
         
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
      SendToPlayer(playerId, enterResBuffer);
   if (entitiesSpawnBuffer)
      SendToPlayer(playerId, entitiesSpawnBuffer);
   if (playerInitBuffer)
      SendToPlayer(playerId, playerInitBuffer);
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
      
      NotifyAim(playerId, it->second.pawnObjectId, playerPawn->IsAiming());
   }
   
   return true;
}

bool Room::HandleFire(PlayerId playerId, const se::game::C_FireReq& pkt)
{
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
      if (!attackSucc) {
         consoleLogger->Log(Color::Yellow, L"[Room] Attack failed for playerId %u with weaponId %u\n", playerId, pkt.weapon_id());
         return true;   // 공격 시도 실패 (예: 탄약 부족, 재장전 중 등)
      }
      
      NotifyFire(playerId, it->second.pawnObjectId, FireEvent{attackReq.weaponId, attackReq.shotSeed, attackReq.origin, attackReq.direction});
   }
   
   return true;
}

bool Room::HandleThrowGrenade(PlayerId playerId, const se::game::C_ThrowGrenadeReq& pkt)
{
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
      
      const uint32 grenadeType = pkt.grenade_type();
      const auto& startPos = pkt.start_position();
      const auto& dir = pkt.direction();
      
      const Vector3 position = Vector3{startPos.x(), startPos.y(), startPos.z()};
      
      InventoryOpResult grenadeResult = playerPawn->ConsumeItem(grenadeType, 1, ItemChangeContext(ItemChangeReason::Consume));
      if (!grenadeResult.accepted) {
         consoleLogger->Log(Color::Yellow, L"[Room] Failed to consume grenade item for playerId %u. ItemId: %u\n", playerId, grenadeType);
         return true;   // 수류탄 아이템 소비 실패 (예: 인벤토리에 수류탄이 없음)
      }
      
      GrenadeActor* grenade = SpawnObject<GrenadeActor>(ObjectFlags::None);
      if (!grenade) {
         consoleLogger->Log(Color::Yellow, L"[Room] Failed to spawn GrenadeActor for playerId %u\n", playerId);
         return true;   // 수류탄 액터 생성 실패 (정상적이지 않은 상황)
      }
      grenade->Init(playerPawn->GetId(), position, Vector3{}, 100, 0, 10.0f, 450.0f, true);
      ObjectId grenadeId = grenade->GetId();
      
      NotifyThrowGrenade(it->second.pawnObjectId, grenadeId, grenadeType, position, Vector3{dir.x(), dir.y(), dir.z()});
   }
   
   return true;
}

bool Room::HandleReload(PlayerId playerId, const se::game::C_ReloadReq& pkt)
{
   // TODO: 재장전 결과 패킷 (S_ReleadRes 를 작성해서 프로토콜 업데이트 하기)
   SendBufferRef reloadResultBuffer;      // 재장전 결과를 해당 플레이어에게 보내는 패킷 (예: 재장전 성공 여부, 남은 탄창 수 등)
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

      const uint32 weaponId = pkt.weapon_id();
      const uint32 handWeaponId = playerCombatComp->GetCurrentWeaponId();
      if (handWeaponId != weaponId) {
         consoleLogger->Log(Color::Yellow, L"[Room] Reload Failed: weapon_id mismatch (handWeaponId: %u, pkt.weapon_id: %u)\n", handWeaponId, pkt.weapon_id());
      }
      playerCombatComp->TryReload();
      
      NotifyReload(playerId, it->second.pawnObjectId, weaponId);
   }
   
   // TODO: 여기서 보낼 게 아니다 (재장전 완료 시간에 보내야 한다)
   if (reloadResultBuffer)
      SendToPlayer(playerId, reloadResultBuffer);   // 재장전 결과를 해당 플레이어에게 전송
   
   return true;
}

bool Room::HandleWeaponChange(PlayerId playerId, const se::game::C_WeaponChangeReq& pkt)
{
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
      NotifyWeaponChange(playerId, it->second.pawnObjectId, handWeaponId);
   }
   
   return true;
}

bool Room::HandleGrenadeMoveSync(PlayerId playerId, const se::game::C_GrenadeMoveSyncReq& pkt)
{
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
      
      ObjectId grenadeId{pkt.entity_id().value() };
      auto* grenade = objectManager_.FindAs<GrenadeActor>(grenadeId);
      if (!grenade) {
         consoleLogger->Log(Color::Yellow, L"[Room] GrenadeActor not found for grenadeId %u during grenade move sync\n", pkt.entity_id().value());
         return false;
      }
      
      const auto& position = pkt.position();
      const auto& rotation = pkt.rotation();
      const auto& velocity = pkt.velocity();
      
      const Vector3 grenadePos{position.x(), position.y(), position.z()};
      const Vector3 grenadeRot{rotation.yaw(), rotation.pitch(), rotation.roll()};
      const Vector3 grenadeVel{velocity.x(), velocity.y(), velocity.z()};
      
      grenade->SetPosition(grenadePos);
      
      NotifyGrenadeMoveSync(playerId, grenadeId, grenadePos, grenadeRot, grenadeVel);
   }
   
   return true;
}

bool Room::HandleGrenadeExplosion(PlayerId playerId, const se::game::C_GrenadeExplosionReq& pkt)
{
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
      
      ObjectId grenadeId{pkt.entity_id().value() };
      const auto& position = pkt.position();
      const Vector3 requestPos{position.x(), position.y(), position.z()};
      
      auto* grenade = objectManager_.FindAs<GrenadeActor>(grenadeId);
      if (!grenade) {
         consoleLogger->Log(Color::Yellow, L"[Room] GrenadeActor not found for grenadeId %u during grenade explosion handling\n", pkt.entity_id().value());
         return false;
      }
      
      const Vector3 resolvedPos = ValidateGrenadeExplosionPosition(*grenade, requestPos);
      
      grenade->SetPosition(resolvedPos);
      grenade->Explode(GetObjectManager());
      
      // NotifyGrenadeExplosion(playerId, grenadeId, resolvedPos);     // 고민 해보기 (비쥬얼 적인 것과 논리적인 것을 완벽하게 일치시켜야 하는가?)
      NotifyGrenadeExplosion(playerId, grenadeId, requestPos);
   }
   
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
      if (playerPawn->GetItemCount(itemId) <= 0)
         return false;  // 인벤토리에 해당 아이템이 없는 경우 (정상적이지 않은 상황)
      
      // TODO: 임시로 즉시 사용 (나중에 사용 모션을 적용할 경우 여기서 사용 모션 시작 처리하고, 실제 아이템 효과 적용은 모션이 끝나는 시점으로 변경하기)
      //       Timer 사용해서 일정 시간 후에 아이템 효과 적용하는 구조로 변경하기 (예: 회복 아이템 사용 시 사용 모션이 2초 걸린다면, 2초 후에 아이템 효과가 적용되도록)
      InventoryOpResult result = playerPawn->ConsumeItem(itemId, 1, ItemChangeContext(ItemChangeReason::Consume));
      if (!result.accepted) {
         return false;
      }
      
      // TODO: 개선하기 (현재는 itemId로 아이템 종류 구분, 나중에 Item Data로 관리하기)
      switch (itemId)
      {
      case 8:     // 작은 회복약
         playerPawn->Heal(20);
         break;
      case 9:     // 큰 회복약
         playerPawn->Heal(60);
         break;
      case 10:    // 스킬 부스트
         // 스킬 쿨타임 감소 (그 뭐냐 Save Point 하는 거)
         break;
      }
      
      // TODO: 아이템 사용 처리 로직 (예: 아이템 효과 적용, 인벤토리에서 아이템 제거 등)
      //       유효 하다면 다음으로
      //       그리고 여기서 만족하는 게 아니라 배그와 같이 사용하는 모션동안 사용이 취소될 수 있는 구조로 변경하기 (예: 회복 아이템 사용 중에 캔슬하면 아이템이 사용되지 않도록)
      
      NotifyUseItem(playerId, it->second.pawnObjectId, itemId);
   }
   
   return true;
}

bool Room::HandleChestInteract(PlayerId playerId, const se::game::C_ChestInteractReq& pkt)
{
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
         
         constexpr int32 chestMoneyReward = 100;
         playerPawn->AddMoney(CurrencyType::TimePoint, chestMoneyReward, MoneyChangeContext{MoneyChangeReason::Loot});
         
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
         
         NotifyChestInteract(playerId, it->second.pawnObjectId, chestId);
      }
   }
   
   return true;
}

bool Room::HandlePickupItem(PlayerId playerId, const se::game::C_PickupItemReq& pkt)
{
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
      
      auto* item = dynamic_cast<WorldItemActor*>(itemObj);
      if (!item)
         return false;
      
      const auto& itemStack = item->GetItemStack();
      if (!itemStack.IsValid())
         return false;
      
      playerPawn->AddItem(itemStack.id, itemStack.count, ItemChangeContext(ItemChangeReason::Loot));
      
      NotifyPickupItem(pawnId, itemObjectId);

      HandleDespawn(itemObjectId);
   }
   
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
         else {
            res.set_store_item_id(result.entryId);
            res.set_new_price(result.newCost);
            res.set_is_sold_out(result.slotOff);
         }
      }
      
      useStoreResultBuffer = ServerPacketHandler::MakeSendBuffer(res);
   }
   
   if (useStoreResultBuffer)
      SendToPlayer(playerId, useStoreResultBuffer);   // 상점 이용 결과를 해당 플레이어에게 전송
   
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
      SendToPlayer(playerId, setSavePointResultBuffer); // 세이브 포인트 설정 결과를 해당 플레이어에게 전송
   
   return true;
}

bool Room::HandleJump(PlayerId playerId, const se::game::C_JumpReq& pkt)
{
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
      
      playerPawn->SetJumping(true);
      NotifyJump(playerId, it->second.pawnObjectId);
   }
   
   return true;
}

bool Room::HandleJumpLand(PlayerId playerId, const se::game::C_JumpLand& pkt)
{
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
      
      playerPawn->SetJumping(false);
      playerPawn->SetDoubleJumping(false);
      NotifyJumpLand(playerId, it->second.pawnObjectId);
   }
   
   return true;
}

bool Room::HandleDoubleJump(PlayerId playerId, const se::game::C_DoubleJumpReq& pkt)
{
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
      
      playerPawn->SetJumping(false);
      playerPawn->SetDoubleJumping(true);
      NotifyDoubleJump(playerId, it->second.pawnObjectId);
   }
   
   return true;
}

bool Room::HandleCrouch(PlayerId playerId, const se::game::C_CrouchReq& pkt)
{
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
      
      playerPawn->SetCrouching(pkt.is_crouching());
      NotifyCrouch(playerId, it->second.pawnObjectId, pkt.is_crouching());
   }
   
   return true;
}

bool Room::HandleWireAction(PlayerId playerId, const se::game::C_WireActionReq& pkt)
{
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      const auto& anchorPoint = pkt.anchor_point();
      
      playerPawn->SetWired(true);
      NotifyWireAction(playerId, it->second.pawnObjectId, {anchorPoint.x(), anchorPoint.y(), anchorPoint.z()});
   }
   
   return true;
}

bool Room::HandleWireActionEnd(PlayerId playerId, const se::game::C_WireActionEnd& pkt)
{
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
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
      NotifyWireEnd(playerId, it->second.pawnObjectId);
   }
   
   return true;
}

bool Room::HandleWireLaunch(PlayerId playerId, const se::game::C_WireLaunchReq& pkt)
{
   std::shared_ptr<PlayerSession> sessionRef = sessionManager_.FindByPlayerId(playerId);
   
   if (!sessionRef) return false;   // 세션이 존재하지 않음 (정상적이지 않은 상황)
   
   {
      if (playerId == 0)
         return false;
      
      auto it = roomPlayers_.find(playerId);
      if (it == roomPlayers_.end())
         return false;
      
      auto* playerPawn = objectManager_.FindAs<PlayerPawn>(it->second.pawnObjectId);
      if (!playerPawn)
         return false;
      
      const auto& startPos = pkt.start_position();
      const auto& direction = pkt.direction();
      
      NotifyWireLaunch(playerId, it->second.pawnObjectId, {startPos.x(), startPos.y(), startPos.z()}, {direction.x(), direction.y(), direction.z()});
   }
   
   return true;
}

bool Room::HandleEquipItem(PlayerId playerId, const se::game::C_EquipItemReq& pkt)
{
   SendBufferRef equipItemResultBuffer;
   
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
      
      bool hasItem = (playerPawn->GetItemCount(itemId) >= 1);
      
      
      if (not hasItem) {
         // 인벤토리에 아이템이 없는 경우 (정상적이지 않은 상황)
         consoleLogger->Log(Color::Yellow, L"[Room] Equip Item Failed: Player does not have itemId %u in inventory\n", itemId);
      }
      else {
         NotifyEquipItem(playerId, it->second.pawnObjectId, itemId);
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
      SendToPlayer(playerId, equipItemResultBuffer);  // 아이템 장착 결과를 해당 플레이어에게 전송
   
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
      SendToPlayer(playerId, skillEquipResultBuffer);   // 스킬 장착 결과를 해당 플레이어에게 전송
   
   return false;
}

// TODO: Monster 우선 만들고 아래 핸들러 구현하기 (Monster Spawn, Monster Attack 등)
bool Room::HandleSpawnMonster(PlayerId playerId, const se::test::C_SpawnMonsterReq& pkt)
{
   const auto& pos = pkt.spawn_position();
   return SpawnMonster(Vector3{pos.x(), pos.y(), pos.z()}, pkt.enemy_type());    // TEMP: Table ID는 클라이언트에서 모른다
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
   health.SetMaxHpUnsafe(static_cast<int32>(pkt.max_health()));
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
   item->SetItemStack(params.itemStack);
   ReplicationSpawn(item, params.itemStack.id, item->GetYaw(), params.itemStack.count);
   return item;
}

StoreActor* Room::CreateStoreActor(const Vector3& pos, float yaw)
{
   auto* store = SpawnObject<StoreActor>(ObjectFlags::None);
   if (!store)
      return nullptr;

   store->SetTransform(pos, yaw);
   return store;
}

ChestActor* Room::CreateChestActor(const Vector3& pos, int32 tableId, float yaw)
{
   auto* chest = SpawnObject<ChestActor>(ObjectFlags::None);
   if (!chest)
      return nullptr;

   chest->SetTransform(pos, yaw);
   chest->SetTableId(tableId);
   return chest;
}

MonsterPawn* Room::CreateMonsterActor(const Vector3& pos, uint32 templateId, float yaw)
{
   auto* monster = SpawnObject<MonsterPawn>(ObjectFlags::Replicable | ObjectFlags::Tickable, templateId);
   if (!monster)
      return nullptr;
   
   monster->SetTransform(pos, yaw);
   monster->SetSavedRespawnPosition(monster->GetPosition());
   return monster;
}

bool Room::SpawnMonster(const Vector3& pos, uint32 templateId, float yaw)
{
   MonsterPawn* monster = CreateMonsterActor(pos, templateId, yaw);
   if (!monster)
      return false;
   
   monster->StartAI();
   
   ReplicationSpawn(monster, templateId, monster->GetYaw());
   return true;
}

bool Room::SpawnChest(const Vector3& pos, int32 tableId, float yaw)
{
   ChestActor* chest = CreateChestActor(pos, tableId, yaw);
   if (!chest)
      return false;
   
   ReplicationSpawn(chest, /*templateId=*/0, chest->GetYaw());    // TODO: templateId는 나중에 생각하기
   
   return true;
}

bool Room::SpawnStore(const Vector3& pos, float yaw)
{
   StoreActor* store = CreateStoreActor(pos, yaw);
   if (!store)
      return false;
   
   ReplicationSpawn(store, /*templateId=*/0, store->GetYaw());    // TODO: templateId는 나중에 생각하기
   
   return true;
}

bool Room::Start()
{
   if (roomState_ != RoomState::Loading) 
      return false;
   
   if (!roomGameSystem_.Start())
      return false;
   
   objectManager_.ForEachAlive([](BaseObject* obj)
   {
      MonsterPawn* monsterPawn = dynamic_cast<MonsterPawn*>(obj);
      if (monsterPawn) {
         monsterPawn->StartAI();
      }
   });
   
   roomState_ = RoomState::Playing;
   if (ownerShard_)
      ownerShard_->ScheduleRoomFirstTick(roomId_);
   
   BroadcastGameStart();
   return true;
}

void Room::Tick(const RepFrame& frame)
{
   // Room 정책
   // GameSystem 진행
   // Object Tick 진행
   const float deltaSeconds = frame.dt.count() / 1000.0f;
   lastDeltaTime_ = deltaSeconds;
   
   roomGameSystem_.Update(deltaSeconds);

   objectManager_.ForEachTickableAlive([deltaSeconds](BaseObject* obj)
   {
      obj->__Tick(deltaSeconds);
   });
   
   // objectManager_.SweepDestroy();   // 오브젝트 제거 처리

   roomGameSystem_.GetReplicationSystem().FlushImmediate(frame);
   roomGameSystem_.GetReplicationSystem().FlushPeriodic(frame);
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

void Room::Broadcast(SendBufferRef sendBuffer, PlayerId exceptPlayerId)
{
   if (not sendBuffer)
      return;   // 유효하지 않은 SendBuffer

   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (playerId == exceptPlayerId)
         continue;   // 제외할 플레이어는 건너뛰기
      
      if (roomPlayer.sessionId == 0)
         continue;   // 유효하지 않은 세션 ID인 플레이어는 건너뛰기
      
      if (auto session = sessionManager_.FindBySessionId(roomPlayer.sessionId)) {
         if (session->GetState() == PlayerSessionState::Closing or session->GetState() == PlayerSessionState::Closed)
            continue;  // 세션이 유효한 상태가 아님 (세션이 닫히는 중이거나 이미 닫힌 상태)
         
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
   
   if (session->GetState() == PlayerSessionState::Closing or session->GetState() == PlayerSessionState::Closed)
      return false;  // 세션이 유효한 상태가 아님 (세션이 닫히는 중이거나 이미 닫힌 상태)
   
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

void Room::BroadcastGameEnd(PlayerId winnerPlayerId)
{
   se::game::N_GameEnd noti;
   
   SendBufferRef gameEndBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   if (gameEndBuffer)
      Broadcast(gameEndBuffer);   // 모두에게 게임 종료 정보 Broadcast
}

void Room::BroadcastRoomClose()
{
   se::room::N_RoomClosed noti;
   {
      noti.set_room_id(roomId_);
      noti.set_reason(se::room::ROOM_CLOSED_REASON_GAME_FINISHED);
   }
   
   SendBufferRef roomCloseBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   
   if (roomCloseBuffer)
      Broadcast(roomCloseBuffer);
}

void Room::NotifyPlayerGameResult(PlayerId playerId, uint32 rank, int32 score, PlayerId killerId)
{
   se::game::N_PlayerGameResult noti;
   {
      noti.set_rank(rank);
      noti.set_score(score);
      
      if (killerId != 0) {
         if (roomPlayers_.contains(killerId)) {
            noti.set_killer(roomPlayers_[killerId].nickname);
         }
      }
   }
   
   SendBufferRef gameResultBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   
   if (gameResultBuffer)
      SendToPlayer(playerId, gameResultBuffer);
}

void Room::NotifyAim(PlayerId playerId, ObjectId pawnId, bool isAiming)
{
   RepEvent aimEvent;
   ReplicateEventSet(aimEvent, RepEventType::Aim);
   aimEvent.header.source = pawnId;
   aimEvent.header.exceptPlayerId = playerId;
   aimEvent.payload = AimEvent{isAiming};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(aimEvent);
}

void Room::NotifyFire(PlayerId playerId, ObjectId pawnId, const FireEvent& fireEvent)
{
   RepEvent fireRepEvent;
   
   ReplicateEventSet(fireRepEvent, RepEventType::Fire);
   fireRepEvent.header.source = pawnId;
   fireRepEvent.header.exceptPlayerId = playerId;
   fireRepEvent.payload = fireEvent;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(fireRepEvent);
}

void Room::NotifyReload(PlayerId playerId, ObjectId pawnId, uint32 weaponId)
{
   RepEvent reloadEvent;
   ReplicateEventSet(reloadEvent, RepEventType::Reload);
   reloadEvent.header.source = pawnId;
   reloadEvent.header.exceptPlayerId = playerId;
   reloadEvent.payload = ReloadEvent{weaponId};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(reloadEvent);
}

void Room::NotifyWeaponChange(PlayerId playerId, ObjectId pawnId, uint32 newWeaponId)
{
   RepEvent weaponChangeEvent;
   ReplicateEventSet(weaponChangeEvent, RepEventType::WeaponChange);
   weaponChangeEvent.header.source = pawnId;
   weaponChangeEvent.header.exceptPlayerId = playerId;
   weaponChangeEvent.payload = WeaponChangedEvent{newWeaponId};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(weaponChangeEvent);
}

void Room::ReplicationDespawn(ObjectId objectId)
{
   RepEvent despawnEvent;
   ReplicateEventSet(despawnEvent, RepEventType::Despawn);
   despawnEvent.header.source = objectId;
   roomGameSystem_.GetReplicationSystem().PushEvent(despawnEvent);
}

void Room::ReplicationDeath(ObjectId objectId)
{
   RepEvent deathEvent;
   ReplicateEventSet(deathEvent, RepEventType::Death);
   deathEvent.header.source = objectId;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(deathEvent);
}

void Room::ReplicationRespawn(ObjectId objectId)
{
   Pawn* pawn = objectManager_.FindAs<Pawn>(objectId);
   if (pawn == nullptr)
      return;   // 유효하지 않은 Pawn 객체
   
   const Vector3& respawnPos = pawn->GetSavedRespawnPosition();
   const float respawnYaw = pawn->GetYaw();
   
   RepEvent respawnEvent;
   ReplicateEventSet(respawnEvent, RepEventType::Respawn);
   respawnEvent.header.source = objectId;
   respawnEvent.payload = RespawnEvent{respawnPos, respawnYaw};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(respawnEvent);
}

void Room::ReplicationKillPlayer(ObjectId killerId, ObjectId victimId)
{
   RepEvent killPlayerEvent;
   ReplicateEventSet(killPlayerEvent, RepEventType::KillPlayer);
   killPlayerEvent.header.source = killerId;
   killPlayerEvent.header.target = victimId;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(killPlayerEvent);
}

void Room::ReplicationZoneChange(uint32 phase, const ZoneCircle& newZone, float waitDuration, float shrinkDuration)
{
   RepEvent zoneChangeEvent;
   ReplicateEventSet(zoneChangeEvent, RepEventType::ZoneChange);
   zoneChangeEvent.payload = ZoneChangeEvent{newZone.center, newZone.radius, waitDuration, shrinkDuration};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(zoneChangeEvent);
}

void Room::NotifyHit(ObjectId objectId, const SE::Math::Vector3& point, int32 damage)
{
   RepEvent entityHitEvent;
   ReplicateEventSet(entityHitEvent, RepEventType::Hit);
   entityHitEvent.header.source = objectId;
   entityHitEvent.payload = HitEvent{point, damage};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(entityHitEvent);
}

void Room::NotifyProjectileSpawn(ProjectileActor* projectile, uint32 templateId)
{
   if (!projectile)
      return;   // 유효하지 않은 발사체
   
   ReplicationSpawn(projectile, templateId, projectile->GetYaw());
}

void Room::NotifyPickupItem(ObjectId playerObjectId, ObjectId itemObjectId)
{
   RepEvent pickupItemEvent;
   ReplicateEventSet(pickupItemEvent, RepEventType::PickupItem);
   pickupItemEvent.header.source = playerObjectId;
   pickupItemEvent.header.target = itemObjectId;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(pickupItemEvent);
}

void Room::NotifyUseItem(PlayerId playerId, ObjectId playerObjectId, uint32 itemId)
{
   RepEvent useItemEvent;
   ReplicateEventSet(useItemEvent, RepEventType::UseItem);
   // useItemEvent.header.exceptPlayerId = playerId;     // 그냥 기다렸다 패킷 받으면 애니메이션 재생
   useItemEvent.header.source = playerObjectId;
   useItemEvent.payload = UseItemEvent{itemId};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(useItemEvent);
}

void Room::NotifyChestInteract(PlayerId playerId, ObjectId pawnId, ObjectId chestId)
{
   RepEvent chestInteractEvent;
   ReplicateEventSet(chestInteractEvent, RepEventType::ChestInteract);
   // chestInteractEvent.header.exceptPlayerId = playerId;   // 그냥 기다렸다 패킷 받으면 애니메이션 재생
   chestInteractEvent.header.source = pawnId;
   chestInteractEvent.header.target = chestId;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(chestInteractEvent);
}

void Room::NotifyJump(PlayerId playerId, ObjectId pawnId)
{
   RepEvent jumpEvent;
   ReplicateEventSet(jumpEvent, RepEventType::Jump);
   jumpEvent.header.exceptPlayerId = playerId;
   jumpEvent.header.source = pawnId;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(jumpEvent);
}

void Room::NotifyJumpLand(PlayerId playerId, ObjectId pawnId)
{
   RepEvent jumpLandEvent;
   ReplicateEventSet(jumpLandEvent, RepEventType::JumpLand);
   jumpLandEvent.header.exceptPlayerId = playerId;
   jumpLandEvent.header.source = pawnId;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(jumpLandEvent);
}

void Room::NotifyDoubleJump(PlayerId playerId, ObjectId pawnId)
{
   RepEvent doubleJumpEvent;
   ReplicateEventSet(doubleJumpEvent, RepEventType::DoubleJump);
   doubleJumpEvent.header.exceptPlayerId = playerId; 
   doubleJumpEvent.header.source = pawnId;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(doubleJumpEvent);
}

void Room::NotifyCrouch(PlayerId playerId, ObjectId pawnId, bool isCrouching)
{
   RepEvent crouchEvent;
   ReplicateEventSet(crouchEvent, RepEventType::Crouch);
   crouchEvent.header.exceptPlayerId = playerId;
   crouchEvent.header.source = pawnId;
   crouchEvent.payload = CrouchEvent{isCrouching};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(crouchEvent);
}

void Room::NotifyWireLaunch(PlayerId playerId, ObjectId pawnId, const Vector3& startPos, const Vector3& direction)
{
   RepEvent wireLaunchEvent;
   ReplicateEventSet(wireLaunchEvent, RepEventType::WireLaunch);
   wireLaunchEvent.header.exceptPlayerId = playerId;
   wireLaunchEvent.header.source = pawnId;
   wireLaunchEvent.payload = WireLaunchEvent{startPos, direction};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(wireLaunchEvent);
}

void Room::NotifyWireAction(PlayerId playerId, ObjectId pawnId, const Vector3& anchorPoint)
{
   RepEvent wireActionEvent;
   ReplicateEventSet(wireActionEvent, RepEventType::WireAction);
   wireActionEvent.header.exceptPlayerId = playerId;
   wireActionEvent.header.source = pawnId;
   wireActionEvent.payload = WireActionEvent{anchorPoint};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(wireActionEvent);
}

void Room::NotifyWireEnd(PlayerId playerId, ObjectId pawnId)
{
   RepEvent wireEndEvent;
   ReplicateEventSet(wireEndEvent, RepEventType::WireActionEnd);
   wireEndEvent.header.exceptPlayerId = playerId;
   wireEndEvent.header.source = pawnId;
   
   roomGameSystem_.GetReplicationSystem().PushEvent(wireEndEvent);
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

void Room::NotifyEquipItem(PlayerId playerId, ObjectId pawnId, uint32 itemId)
{
   RepEvent equipItemEvent;
   ReplicateEventSet(equipItemEvent, RepEventType::EquipItem);
   equipItemEvent.header.exceptPlayerId = playerId;
   equipItemEvent.header.source = pawnId;
   equipItemEvent.payload = EquipItemEvent{itemId};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(equipItemEvent);
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

void Room::NotifyWeaponStatChange(PlayerId id, uint32 weaponId, const WeaponStatModifier& newStat)
{
   RepEvent weaponStatChangeEvent;
   ReplicateEventSet(weaponStatChangeEvent, RepEventType::WeaponStatChange);
   weaponStatChangeEvent.header.playerId = id;
   weaponStatChangeEvent.payload = WeaponStatChangeEvent{weaponId, newStat};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(weaponStatChangeEvent);
}

void Room::NotifyZoneFlow(bool flowing)
{
   RepEvent zoneFlowEvent;
   ReplicateEventSet(zoneFlowEvent, RepEventType::ZoneFlow);
   zoneFlowEvent.payload = ZoneFlowEvent{flowing};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(zoneFlowEvent);
}

void Room::NotifyExplosion(ObjectId sourceId, ObjectId ownerId, const Vector3& pos, float radius)
{
   auto* ownerPawn = objectManager_.FindAs<Pawn>(ownerId);
   if (!ownerPawn)
      return;   // 유효하지 않은 폭발 소유자
   
   PlayerId ownerPlayerId = ownerPawn->GetOwnerPlayerId();
   
   // 폭발 이벤트를 생성하여 클라이언트에게 전송 (폭발 이펙트 재생을 위해)
   RepEvent explosionEvent;
   ReplicateEventSet(explosionEvent, RepEventType::Explosion);
   explosionEvent.header.playerId = ownerPlayerId;
   explosionEvent.header.source = sourceId;
   explosionEvent.payload = ExplosionEvent{pos, radius};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(explosionEvent);
}

void Room::NotifyCombatEvent(ObjectId objectId, CombatEventType combatEven)
{
   RepEvent combatEvent;
   ReplicateEventSet(combatEvent, RepEventType::Attack);
   combatEvent.header.source = objectId;
   combatEvent.payload = AttackEvent{static_cast<uint32>(combatEven)};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(combatEvent);
}

void Room::NotifyMonsterFire(ObjectId monsterId, CombatEventType eventType, const Vector3& origin,
   const Vector3& direction, float range)
{
   RepEvent monsterFireEvent;
   ReplicateEventSet(monsterFireEvent, RepEventType::MonsterFire);
   monsterFireEvent.header.source = monsterId;
   monsterFireEvent.payload = MonsterFireEvent{static_cast<uint32>(eventType), origin, direction, range};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(monsterFireEvent);
}

void Room::NotifyThrowGrenade(ObjectId ownerId, ObjectId grenadeId, uint32 grenadeType, const Vector3& pos,
   const Vector3& dir)
{
   RepEvent throwGrenadeEvent;
   ReplicateEventSet(throwGrenadeEvent, RepEventType::GrenadeThrow);
   throwGrenadeEvent.header.source = ownerId;
   throwGrenadeEvent.header.target = grenadeId;
   throwGrenadeEvent.payload = GrenadeThrowEvent{grenadeType, pos, dir};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(throwGrenadeEvent);
}

void Room::NotifyGrenadeMoveSync(PlayerId ownerId, ObjectId grenadeId, const Vector3& newPos, const Vector3& newRotate,
                                 const Vector3& newVel)
{
   RepEvent grenadeMoveEvent;
   ReplicateEventSet(grenadeMoveEvent, RepEventType::GrenadeMoveSync);
   grenadeMoveEvent.header.source = grenadeId;
   grenadeMoveEvent.header.exceptPlayerId = ownerId;
   grenadeMoveEvent.payload = GrenadeMoveSyncEvent{newPos, newRotate, newVel};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(grenadeMoveEvent);
}

void Room::NotifyGrenadeExplosion(PlayerId ownerId, ObjectId grenadeId, const Vector3& exPos)
{
   RepEvent grenadeExplosionEvent;
   ReplicateEventSet(grenadeExplosionEvent, RepEventType::GrenadeExplosion);
   grenadeExplosionEvent.header.source = grenadeId;
   grenadeExplosionEvent.header.exceptPlayerId = ownerId;
   grenadeExplosionEvent.payload = GrenadeExplosionEvent{exPos};
   
   roomGameSystem_.GetReplicationSystem().PushEvent(grenadeExplosionEvent);
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

void Room::HandleDamageResult(Pawn* attacker, Pawn* victim, const DamageResult& damageResult)
{
   if (!attacker || !victim)
      return;   // 유효하지 않은 공격자 또는 피해자
   
   if (!damageResult.killed)
      return;
   
   const bool KillerIsPlayer = attacker->IsPlayer();
   const bool VictimIsPlayer = victim->IsPlayer();
   
   if (KillerIsPlayer and VictimIsPlayer) {
      HandlePlayerKillPlayer(attacker, victim);   // Player Kill 처리 (예: 킬 스코어 증가, 킬 보너스 지급 등)
      return;
   }
   
   if (KillerIsPlayer and victim->IsMonster()) {
      HandlePlayerKillMonster(attacker, victim);     // Player가 NPC에게 죽인 경우 처리)
      return;
   }
   
   if (attacker->IsMonster() and VictimIsPlayer) {
      // Player가 NPC에게 죽은 경우 처리...
      return;
   }
}

void Room::HandlePawnDeath(Pawn* pawn, const DamageContext& ctx, const DamageResult& damageResult)
{
   if (!pawn)
      return;  // 유효하지 않은 Pawn
   
   Pawn* attacker = nullptr;
   
   if (ctx.attacker != ObjectId{}) {
      attacker = objectManager_.FindAs<Pawn>(ctx.attacker);
   }
   
   if (attacker == nullptr and ctx.instigator != ObjectId{}) {
      attacker = objectManager_.FindAs<Pawn>(ctx.instigator);
   }
   
   ReplicationDeath(pawn->GetId());
   
   if (attacker) {
      HandleDamageResult(attacker, pawn, damageResult);
   }
   
   roomGameSystem_.OnPawnDeath(pawn->GetId());
}

void Room::HandlePawnRespawn(ObjectId pawnId)
{
   ReplicationRespawn(pawnId);
}

void Room::HandleDespawn(ObjectId objId)
{
   if (DespawnObject(objId))
      ReplicationDespawn(objId);
}

void Room::HandlePlayerKillPlayer(Pawn* killer, Pawn* victim)
{
   constexpr int32 killRobbery = 100;
   
   const PlayerId killerPlayerId = killer->GetOwnerPlayerId();
   const PlayerId victimPlayerId = victim->GetOwnerPlayerId();
      
   if (killerPlayerId == 0 or victimPlayerId == 0)
      return;   // 유효하지 않은 플레이어 ID (이 경우는 발생하지 않아야 함)
      
   if (!HasPlayer(killerPlayerId) || !HasPlayer(victimPlayerId))
      return;   // 방에 존재하지 않는 플레이어가 공격자 또는 피해자인 경우 (이 경우는 발생하지 않아야 함)
   
   PlayerPawn* killerPlayerPawn = static_cast<PlayerPawn*>(killer);
   PlayerPawn* victimPlayerPawn = static_cast<PlayerPawn*>(victim);
   
   const int32 victimCurrentMoney = victimPlayerPawn->GetBalance(CurrencyType::TimePoint);
   const int32 actualRobbery = std::min(killRobbery, victimCurrentMoney);
   MoneyChangeResult result = victimPlayerPawn->SpendMoney(CurrencyType::TimePoint, actualRobbery, MoneyChangeContext{MoneyChangeReason::DropOnDeath});
   if (result.accepted) {
      MoneyChangeResult addResult = killerPlayerPawn->AddMoney(CurrencyType::TimePoint, actualRobbery, MoneyChangeContext{MoneyChangeReason::Robbery});
   }
   
   const ObjectId killerId = killer->GetId();
   const ObjectId victimId = victim->GetId();
   ReplicationKillPlayer(killerId, victimId);
}

void Room::HandlePlayerKillMonster(Pawn* killer, Pawn* monster)
{
   constexpr int32 killRobbery = 100;
   
   const PlayerId killerPlayerId = killer->GetOwnerPlayerId();
      
   if (killerPlayerId == 0)
      return;   // 유효하지 않은 플레이어 ID (이 경우는 발생하지 않아야 함)
      
   if (!HasPlayer(killerPlayerId))
      return;   // 방에 존재하지 않는 플레이어가 공격자 또는 피해자인 경우 (이 경우는 발생하지 않아야 함)
   
   PlayerPawn* killerPlayerPawn = static_cast<PlayerPawn*>(killer);
   GetRoomGameSystem().GetDropSystem().OnEntityDied(monster->GetId());
   
   MoneyChangeResult addResult = killerPlayerPawn->AddMoney(CurrencyType::TimePoint, killRobbery, MoneyChangeContext{MoneyChangeReason::Robbery});
}

void Room::HandleMonsterFire(ObjectId monsterId, CombatEventType eventType, const SE::Math::Vector3& origin,
                             const SE::Math::Vector3& direction, float range, int32 damage)
{
   if (monsterId == ObjectId{}) {
      return;
   }
   
   if (range <= 0.0f || damage <= 0) {
      return;
   }

   if (direction.LengthSq() <= 0.0001f) {
      return;
   }

   const SE::Math::Vector3 fireDir = direction.Normalized();
   SE::Physics::Ray ray(origin, fireDir, range);

   Actor* victim = nullptr;

   SE::Physics::Hit::HitResult outHit;
   GetRoomGameSystem().GetCombatSystem().TraceHit(ray, monsterId, outHit);
   if (outHit.hit) {
      victim = outHit.actor;
   }
   
   const ObjectId victimId = victim ? victim->GetId() : ObjectId{};
   NotifyMonsterFire(monsterId, eventType, origin, fireDir, range);
   
   if (outHit.hit)
      NotifyHit(victimId, outHit.point, damage);
   
   if (victim == nullptr) 
      return;   // 히트한 Actor가 없는 경우
    
   IDamageable* damageable = dynamic_cast<IDamageable*>(victim);
   if (!damageable)
      return;
    
   DamageContext ctx;
   ctx.attacker = monsterId;
   ctx.type = DamageType::Ranged;
   ctx.source = DamageSource::Weapon;
    
   damageable->ApplyDamage(GetObjectManager(), damage, ctx);
}

void Room::HandleMonsterMelee(const MeleeAttackDesc& desc)
{
   if (desc.collider == nullptr) {
      return;
   }
   
   if (desc.attackerId == ObjectId{}) {
      return;
   }
   
   auto* attacker = GetObjectManager().FindAs<Pawn>(desc.attackerId);
   if (attacker == nullptr || attacker->IsDead()) {
      return;
   }
   
   DamageContext ctx;
   ctx.attacker = desc.attackerId;
   ctx.type = DamageType::Melee;
   ctx.source = DamageSource::Monster;
   
   std::unordered_set<ObjectId> damagedPawns;
   
   GetObjectManager().ForEachAlive([&](BaseObject* obj)
   {
      auto* pawn = dynamic_cast<Pawn*>(obj);
      if (pawn == nullptr) return;
      if (pawn->GetId() == desc.attackerId) return;
      if (pawn->IsDead()) return;
      if (damagedPawns.contains(pawn->GetId())) return;

      if (desc.hitPlayersOnly && pawn->GetObjectType() != ObjectType::OBJ_PLAYER) {
         return;
      }
      
      obj->ForEachCollider([&](ColliderComponent* collider)
      {
         if (damagedPawns.contains(pawn->GetId())) {
            return;
         }

         if (!collider) return;
         if (collider->GetRole() != ColliderRole::Hurtbox) return;

         const SE::Physics::Collider* hitCollider = collider->GetCollider();
         if (!hitCollider) return;

         SE::Physics::CollisionResult collisionResult;
         if (desc.collider->Intersect(*hitCollider, collisionResult) && collisionResult.hit) {
            damagedPawns.insert(pawn->GetId());
            pawn->ApplyDamage(GetObjectManager(), desc.damage, ctx);
         }
      });
   });
}

void Room::OnRealDeath(ObjectId pawnId)
{
   Pawn* pawn = objectManager_.FindAs<Pawn>(pawnId);
   if (!pawn)
      return;   // 유효하지 않은 Pawn 객체
   
   // Player만 확정 죽음이 있을 테니 Type 확인
   if (not pawn->IsPlayer()) 
      return;
   
   GetRoomGameSystem().GetDropSystem().OnEntityDied(pawn->GetId());
   
   RoomPlayer& roomPlayer = roomPlayers_[pawn->GetOwnerPlayerId()];
   roomPlayer.death = true;
   
   // TODO: 최종 사망한 플레이어에게 처리해 주어야 할 일 이 있다면 연기서 처리 (예: 딜량, 최종 공격자 정보 패킷 처리 등등)
   const uint32 remainPlayerCount = RemainAlivePlayerCount();
   const ObjectId killerId = pawn->GetLastKillerId();
   const PlayerId killerPlayerId = GetPlayerIdByObjectId(killerId);
   
   NotifyPlayerGameResult(roomPlayer.playerId, remainPlayerCount + 1, 0, killerPlayerId);
   
   // 게임이 종료 조건에 도달했는지 확인 (최후의 1인 남았는지 등등)
   CheckGameEndCondition();
}

void Room::OnZoneChanged(uint32 phase, const ZoneCircle& newZone, float waitDuration, float shrinkDuration)
{
   ReplicationZoneChange(phase, newZone, waitDuration, shrinkDuration);
}

Room::Vector3 Room::ValidateGrenadeExplosionPosition(const GrenadeActor& grenade, const Vector3& desiredPos) const
{
   const ServerMap& serverMap = GetGameDataManager()->GetServerMap();

   // 아예 내부가 아니면 그대로 사용
   if (!serverMap.IsInsideStaticGeometry(desiredPos))
      return desiredPos;

   const Vector3 prevPos = grenade.GetPosition();
   const Vector3 delta = desiredPos - prevPos;
   const float distance = delta.Length();

   // 멈춰 있거나 이동량이 거의 없는 상태에서 내부에 박힌 경우
   if (distance <= 0.0001f)
   {
      Vector3 pushedPos{};
      if (serverMap.TryPushOutStaticGeometry(desiredPos, pushedPos))
         return pushedPos;

      return prevPos;
   }

   const Vector3 dir = delta / distance;

   constexpr float ExtraRange = 10.0f;
   const float range = distance + ExtraRange;

   SE::Physics::Ray ray(prevPos, dir, range);

   SE::Physics::RaycastHit hit;

   if (serverMap.Raycast(ray, hit) && hit.hit)
   {
      constexpr float PushOutEpsilon = 2.0f;
      return hit.point + hit.normal * PushOutEpsilon;
   }

   // Raycast로 못 찾았으면 내부 위치를 직접 PushOut
   Vector3 pushedPos;
   if (serverMap.TryPushOutStaticGeometry(desiredPos, pushedPos))
      return pushedPos;

   return prevPos;
}

void Room::CheckGameEndCondition()
{
   int32 alivePlayerCount = 0;
   PlayerId lastAlivePlayerId = 0;
   
   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (!roomPlayer.death) {
         ++alivePlayerCount;
         lastAlivePlayerId = playerId;
      }
   }
   
   if (alivePlayerCount > 1)
      return;   // 아직 게임이 끝날 조건이 아님
   
   if (alivePlayerCount == 1) {
      // 최후의 1인 승리 처리
      NotifyPlayerGameResult(lastAlivePlayerId, alivePlayerCount, 0, 0);   // 승자에게 1등 통보 (킬러 정보는 없음)
   }
   else if (alivePlayerCount == 0) {
      // 모두 죽은 경우 처리 (무승부 등)
      // 이건 처리할 게 없다 (OnRealDeath에서 이미 NotifyPlayerGameResult를 통해 결과 통보가 끝났으므로)
   }
   
   ReserveRoomClose();
}

void Room::ReserveRoomClose()
{
   GameShard* ownerShard = GetOwnerShard();
   if (!ownerShard) {
      consoleLogger->Log(Color::Red, L"[Room] Failed to reserve room close: Owner shard not found for roomId %u\n", roomId_);
      return;   // 유효하지 않은 GameShard (이 경우는 발생하지 않아야 함)
   }
   
   RoomId roomId = GetRoomId();
   
   closeTimerId_ = ScheduleAfter(Duration{Seconds{30}}, [ownerShard, roomId]()
   {
      ownerShard->CloseRoom(roomId);
   });
}

void Room::CheckRoomCloseCondition()
{
   if (not roomPlayers_.empty())
      return;
   
   ownerShard_->CloseRoom(roomId_);
}

uint32 Room::RemainAlivePlayerCount() const
{
   int32 alivePlayerCount = 0;
   
   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (!roomPlayer.death) {
         ++alivePlayerCount;
      }
   }
   
   return alivePlayerCount;
}

PlayerId Room::GetPlayerIdByObjectId(ObjectId objectId) const
{
   for (const auto& [playerId, roomPlayer] : roomPlayers_) {
      if (roomPlayer.pawnObjectId == objectId) {
         return playerId;
      }
   }
   
   return 0;   // 해당 ObjectId를 가진 플레이어가 없는 경우
}

void Room::IndexObject_OnAdd(BaseObject* object)
{
   if (not object)
      return;   // 유효하지 않은 오브젝트
   
   const ObjectId objectId = object->GetId();
}

void Room::IndexObject_OnRemove(ObjectId objectId)
{
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

#pragma once
#include <mutex>
#include "RoomState.h"
#include "Content/Gameplay/Combat/CombatTypes.h"
#include "Content/Object/ObjectId.h"
#include "Content/Gameplay/Combat/DamageTypes.h"
#include "Content/Object/ObjectManager.h"
#include "Data/Navigation/ServerNavigation.h"
#include "Generated/ServerPacketHandler.h"
#include "Network/ServerConfig.h"
#include "Systems/RoomGameSystem.h"
#include "Utils/Random/WeightedRandom.h"

enum class CombatEventType;
class ChestActor;
class GrenadeActor;
class MonsterPawn;
class PlayerSession;
class StoreActor;
struct SpawnWorldItemParams;
class WorldItemActor;
class PlayerPawn;
class GameShard;
class GameDataManager;
class SessionManager;
struct DamageContext;
struct SkillDef;
class Pawn;
class BaseObject;
class ObjectManager;
class Player;
struct PlacementTransform;
struct MonsterSpawnGroupPlacement;
struct ObjectId;

/*---------
   Room
---------*/
//
// Room는 게임 플레이어들이 함께 상호작용하는 공간을 나타냅니다. 
// 각 Room은 고유한 ID를 가지며, 플레이어와 게임 오브젝트들이 존재할 수 있습니다. 
// Room은 게임 로직과 상태를 관리하며, 플레이어 간의 상호작용을 중개하는 역할을 합니다.
//

class Room : public std::enable_shared_from_this<Room>
{
private:
   friend class ReplicationSystem;
   
public:
   using Vector3 = SE::Math::Vector3;

   struct DebugDrawOptions
   {
      uint32 colorRgba = 0;      // 0xRRGGBBAA, 0이면 클라이언트 기본 색상
      float duration = 0.0f;     // 0이면 클라이언트 기본 지속 시간
      float thickness = 0.0f;    // 0이면 클라이언트 기본 두께
      PlayerId exceptPlayerId = 0;
   };
   
public:
   static std::shared_ptr<Room> Create(RoomId roomId, SessionManager& sessionManager)
   {
      auto room = std::make_shared<Room>(roomId, sessionManager);
      room->PostCreate();
      return room;
   }

public:
   explicit Room(RoomId roomId, SessionManager& sessionManager);
   ~Room();
   
   void PostCreate();
   void Close();
   
public:
   bool Init(GameShard* ownerShard, const GameDataManager& gameDataManager, const GameConfig& gameConfig);
   void SetPlayer(const std::vector<PlayerId>& playerIds, const std::vector<std::string>& playerNames);
   void SetObject();
   
public:
   bool Join(PlayerId playerId, SessionId sessionId);
   bool Leave(PlayerId playerId);
   
   bool UpdateSession(PlayerId playerId, SessionId newSessionId);
   
private:
   void JoinPlayerProcess(PlayerId playerId, PlayerPawn* playerPawn);
   
// Handle Packet
public:
   bool HandleLoadingComplete(PlayerId playerId);
   bool HandleMove(PlayerId playerId, const se::game::C_MoveReq& pkt);
   bool HandleAim(PlayerId playerId, const se::game::C_AimReq& pkt);
   bool HandleFire(PlayerId playerId, const se::game::C_FireReq& pkt);
   // bool HandleAttack(PlayerId playerId, const se::game::C_AttackReq& pkt);
   bool HandleThrowGrenade(PlayerId playerId, const se::game::C_ThrowGrenadeReq& pkt);
   bool HandleReload(PlayerId playerId, const se::game::C_ReloadReq& pkt);
   bool HandleWeaponChange(PlayerId playerId, const se::game::C_WeaponChangeReq& pkt);
   bool HandleGrenadeMoveSync(PlayerId playerId, const se::game::C_GrenadeMoveSyncReq& pkt);
   bool HandleGrenadeExplosion(PlayerId playerId, const se::game::C_GrenadeExplosionReq& pkt);
   bool HandleUseSkill(PlayerId playerId, const se::game::C_UseSkillReq& pkt);
   bool HandleUseItem(PlayerId playerId, const se::game::C_UseItemReq& pkt);
   bool HandleChestInteract(PlayerId playerId, const se::game::C_ChestInteractReq& pkt);
   bool HandlePickupItem(PlayerId playerId, const se::game::C_PickupItemReq& pkt);
   bool HandleUseStore(PlayerId playerId, const se::game::C_UseStoreReq& pkt);
   bool HandleSetSavePoint(PlayerId playerId, const se::game::C_SetSavePointReq& pkt);
   bool HandleJump(PlayerId playerId, const se::game::C_JumpReq& pkt);
   bool HandleJumpLand(PlayerId playerId, const se::game::C_JumpLand& pkt);
   bool HandleDoubleJump(PlayerId playerId, const se::game::C_DoubleJumpReq& pkt);
   bool HandleCrouch(PlayerId playerId, const se::game::C_CrouchReq& pkt);
   bool HandleWireAction(PlayerId playerId, const se::game::C_WireActionReq& pkt);
   bool HandleWireActionEnd(PlayerId playerId, const se::game::C_WireActionEnd& pkt);
   bool HandleWireLaunch(PlayerId playerId, const se::game::C_WireLaunchReq& pkt);
   bool HandleEquipItem(PlayerId playerId, const se::game::C_EquipItemReq& pkt);
   bool HandleSkillEquip(PlayerId playerId, const se::game::C_SkillEquipReq& pkt);
   
// Handle Test Packet
public:
   bool HandleSpawnMonster(PlayerId playerId, const se::test::C_SpawnMonsterReq& pkt);
   bool HandleSpawnChest(PlayerId playerId, const se::test::C_SpawnChestReq& pkt);
   bool HandleSpawnStore(PlayerId playerId, const se::test::C_SpawnStoreReq& pkt);
   bool HandleItem(PlayerId playerId, const se::test::C_ItemReq& pkt);
   bool HandleMoney(PlayerId playerId, const se::test::C_MoneyReq& pkt);
   bool HandleHealth(PlayerId playerId, const se::test::C_HealthReq& pkt);
   bool HandleMaxHealth(PlayerId playerId, const se::test::C_MaxHealthReq& pkt);
   bool HandleZoneStop(PlayerId playerId, const se::test::C_ZoneStopReq& pkt);
   bool HandleZoneStart(PlayerId playerId, const se::test::C_ZoneStartReq& pkt);
   bool HandleZoneReset(PlayerId playerId, const se::test::C_ZoneResetReq& pkt);
   bool HandleZoneDamageOff(PlayerId playerId, const se::test::C_ZoneDamageOffReq& pkt);
   bool HandleZoneDamageOn(PlayerId playerId, const se::test::C_ZoneDamageOnReq& pkt);
   
public:
   template<typename T, typename... Args>
   T* SpawnObject(ObjectFlags flags, Args&&... args)
   {
      T* obj = objectManager_.Create<T>(flags, std::forward<Args>(args)...);
      if (not obj) return nullptr;  // 오브젝트 생성 실패
      
      IndexObject_OnAdd(obj);
      return obj;
   }
   
   bool DespawnObject(ObjectId objectId)
   {
      BaseObject* obj = objectManager_.Find(objectId);
      if (not obj) return false;  // 오브젝트가 존재하지 않음
      
      IndexObject_OnRemove(objectId);
      
      return objectManager_.RequestDestroy(objectId);
   }
   
public:
   WorldItemActor* SpawnItem(const SpawnWorldItemParams& params);
   
private:
   StoreActor* CreateStoreActor(const Vector3& pos, float yaw);
   ChestActor* CreateChestActor(const Vector3& pos, int32 tableId, float yaw);
   MonsterPawn* CreateMonsterActor(const Vector3& pos, uint32 templateId, float yaw);

   bool SpawnMonster(const Vector3& pos, uint32 templateId, float yaw = 0.0f);
   bool SpawnBossMonster(const MonsterSpawnGroupPlacement& monsterGroup);
   void ScheduleBossSpawn(size_t monsterGroupIndex);
   bool SpawnChest(const Vector3& pos, int32 tableId, float yaw = 0.0f);
   bool SpawnStore(const Vector3& pos, float yaw = 0.0f);
   
public:
   bool Start();
   
   void Tick(const RepFrame& frame);
   float GetDelta() const { return lastDeltaTime_; }
   
   bool IsPlaying() const { return roomState_ == RoomState::Playing; }

   void SetDebugDrawCollidersEnabled(bool enabled);
   bool IsDebugDrawCollidersEnabled() const;
   void SetDebugDrawColliderIntervalMs(uint64 intervalMs);
   uint64 GetDebugDrawColliderIntervalMs() const;
   
public:
   TimerId ScheduleAt(TimePoint executeAt, Job job);
   TimerId ScheduleAfter(Duration delay, Job job);
   bool CancelScheduled(TimerId timerId);
   
public:
   std::shared_ptr<Room> GetRoomRef() { return shared_from_this(); }
   RoomId GetRoomId() const { return roomId_; }
   
   ObjectManager& GetObjectManager() { return objectManager_; }
   const ObjectManager& GetObjectManager() const { return objectManager_; }
   
   GameShard* GetOwnerShard() { return ownerShard_; }
   const GameShard* GetOwnerShard() const { return ownerShard_; }
   
   RoomGameSystem& GetRoomGameSystem() { return roomGameSystem_; }
   const RoomGameSystem& GetRoomGameSystem() const { return roomGameSystem_; }

   Random32& GetRandom() { return rng_; }
   const Random32& GetRandom() const { return rng_; }
   
   RoomState GetRoomState() const { return roomState_; }
   void SetRoomState(RoomState state) { roomState_ = state; }
   
   const GameDataManager* GetGameDataManager() const { return gameDataManager_; }
   SE::Nav::ServerNavigation::QueryContext* GetNavigationQueryContext() const;
   const GameConfig& GetGameConfig() const { return gameConfig_; }
   
public:
   TickSeq GetTickSeq() const { return tickSeq_; }
   TickSeq AdvanceTick() { return ++tickSeq_; }
   
public:
   bool HasPlayer(PlayerId playerId) const;
   SessionId GetSessionId(PlayerId playerId) const;
   ObjectId GetObjectId(PlayerId playerId) const;
   
private:
   PlayerPawn* CreatePreparedPlayerPawn(PlayerId playerId, const Vector3& spawnPos);
   
private:
   bool GiveItem(PlayerId playerId, const ItemStack& itemStack);
   bool GiveMoney(PlayerId playerId, int32 amount);
   
   void ExecuteTimeAccel(PlayerId playerId, PlayerPawn& playerPawn, const SkillDef& skillDef, const se::game::C_UseSkillReq& pkt, uint64 nowMs);
   void ExecuteTimeAfterImage(PlayerId playerId, PlayerPawn& playerPawn, const SkillDef& skillDef, const se::game::C_UseSkillReq& pkt, uint64 nowMs);
   void ExecuteTimeRewind(PlayerId playerId, PlayerPawn& playerPawn, const SkillDef& skillDef, const se::game::C_UseSkillReq& pkt, uint64 nowMs);
   
private:
   void Broadcast(SendBufferRef sendBuffer, PlayerId exceptPlayerId = 0);
   bool SendToPlayer(PlayerId playerId, SendBufferRef sendBuffer);
   
public:
   void BroadcastReplication(SendBufferRef sendBuffer, PlayerId exceptPlayerId = 0);
   void SendReplication(PlayerId playerId, SendBufferRef sendBuffer);
   
public:
   void BroadcastGameStart();
   void BroadcastGameEnd(PlayerId winnerPlayerId);
   void BroadcastRoomClose();
   void NotifyPlayerGameResult(PlayerId playerId, uint32 rank, int32 score, PlayerId killerId);
   void NotifyAim(PlayerId playerId, ObjectId pawnId, bool isAiming);
   void NotifyFire(PlayerId playerId, ObjectId pawnId, const FireEvent& fireEvent);
   void NotifyReload(PlayerId playerId, ObjectId pawnId, uint32 weaponId);
   void NotifyWeaponChange(PlayerId playerId, ObjectId pawnId, uint32 newWeaponId);
   void NotifyHit(ObjectId objectId, const SE::Math::Vector3& point, int32 damage);
   void NotifyProjectileSpawn(ProjectileActor* projectile, uint32 templateId);
   void NotifyPickupItem(ObjectId playerObjectId, ObjectId itemObjectId);
   void NotifyItemChange(PlayerId playerId, uint32 itemId, int32 newCount, int32 deltaCount);
   void NotifyEquipItem(PlayerId playerId, ObjectId pawnId, uint32 itemId);
   void NotifyStoreEntryBlock(PlayerId playerId, uint32 entryId, bool blocked);
   void NotifyStoreEntrySnapshot(PlayerId playerId, PlayerPawn* playerPawn);
   void NotifySkillUnlock(PlayerId playerId, uint32 skillId);
   void NotifySkillEquip(PlayerId playerId, uint32 skillId, uint32 slotIndex);
   void NotifyUseItem(PlayerId playerId, ObjectId playerObjectId, uint32 itemId);
   void NotifyChestInteract(PlayerId playerId, ObjectId pawnId, ObjectId chestId);
   void NotifyJump(PlayerId playerId, ObjectId pawnId);
   void NotifyJumpLand(PlayerId playerId, ObjectId pawnId);
   void NotifyDoubleJump(PlayerId playerId, ObjectId pawnId);
   void NotifyCrouch(PlayerId playerId, ObjectId pawnId, bool isCrouching);
   void NotifyWireLaunch(PlayerId playerId, ObjectId pawnId, const Vector3& startPos, const Vector3& direction);
   void NotifyWireAction(PlayerId playerId, ObjectId pawnId, const Vector3& anchorPoint);
   void NotifyWireEnd(PlayerId playerId, ObjectId pawnId);
   void NotifyUseSkill(ObjectId pawnId, const UseSkillEvent& useSkillEvent);
   void NotifyHealthChange(PlayerId id, int newHealth, int deltaHealth);
   void NotifyMaxHealthChange(PlayerId id, int newMaxHealth, int newHealth);
   void NotifyTimePointChange(PlayerId id, int newTimePoint, int deltaTimePoint);
   void NotifyWeaponStatChange(PlayerId id, uint32 weaponId, const WeaponStatModifier& newStat);
   void NotifyZoneFlow(bool flowing);
   void NotifyExplosion(ObjectId sourceId, ObjectId ownerId, const Vector3& pos, float radius);
   void NotifyCombatEvent(ObjectId objectId, CombatEventType combatEvent);
   void NotifyMonsterFire(ObjectId monsterId, CombatEventType eventType, const Vector3& origin, const Vector3& direction, float range);
   void NotifyMonsterImpact(ObjectId monsterId, CombatEventType eventType, const Vector3& position);
   void NotifyThrowGrenade(ObjectId ownerId, ObjectId grenadeId, uint32 grenadeType, const Vector3& pos, const Vector3& dir);
   void NotifyGrenadeMoveSync(PlayerId ownerId, ObjectId grenadeId, const Vector3& newPos, const Vector3& newRotate, const Vector3& newVel);
   void NotifyGrenadeExplosion(PlayerId ownerId, ObjectId grenadeId, const Vector3& exPos);
   void NotifyDebugDrawSphere(const Vector3& position, float radius, const DebugDrawOptions& options = {});
   void NotifyDebugDrawCapsule(const Vector3& pointA, const Vector3& pointB, float radius, const DebugDrawOptions& options = {});
   void NotifyDebugDrawOBB(const Vector3& center, const Vector3& halfExtents, float yaw = 0.0f, float pitch = 0.0f,
                           float roll = 0.0f, const DebugDrawOptions& options = {});
   
private:
   void ReplicateEventSet(RepEvent& ev, RepEventType eventType);
   
   void ReplicationSpawn(Actor* actor, uint32 templateId, float yaw, uint32 amount = 0);
   void ReplicationDespawn(ObjectId objectId);
   
   void ReplicationDeath(ObjectId objectId);
   void ReplicationRespawn(ObjectId objectId);
   
   void ReplicationKillPlayer(ObjectId killerId, ObjectId victimId);
   void ReplicationZoneChange(uint32 phase, const ZoneCircle& newZone, float waitDuration, float shrinkDuration);
   
public:
   void HandleDamageResult(Pawn* attacker, Pawn* victim, const DamageResult& damageResult);
   void HandlePawnDeath(Pawn* pawn, const DamageContext& ctx, const DamageResult& damageResult);
   void HandlePawnRespawn(ObjectId pawnId);
   void HandleDespawn(ObjectId objId);
   
private:
   void HandlePlayerKillPlayer(Pawn* killer, Pawn* victim);
   void HandlePlayerKillMonster(Pawn* killer, Pawn* monster);
   
public:
   void HandleMonsterFire(ObjectId monsterId, CombatEventType eventType, const SE::Math::Vector3& origin, const SE::Math::Vector3& direction, float range, int32 damage);
   void HandleMonsterMelee(const MeleeAttackDesc& meleeAttackDesc);
   
public:
   void OnRealDeath(ObjectId pawnId);
   void OnZoneChanged(uint32 phase, const ZoneCircle& newZone, float waitDuration, float shrinkDuration);
   
private:
   Vector3 ValidateGrenadeExplosionPosition(const GrenadeActor& grenade, const Vector3& desiredPos) const;
   
private:
   void CheckGameEndCondition();
   void ReserveRoomClose();
   void CheckRoomCloseCondition();
   
   uint32 RemainAlivePlayerCount() const;
   
   PlayerId GetPlayerIdByObjectId(ObjectId objectId) const;
   
private:
   void IndexObject_OnAdd(BaseObject* object);
   void IndexObject_OnRemove(ObjectId objectId);
   
private:
   bool AllPlayerJoined() const;
   bool AllPlayerLoaded() const;
   
private:
   void TryTransitToLoading();
   
private:
   SessionManager& sessionManager_;
   GameShard* ownerShard_ = nullptr;   // non-owning
   
private:
   struct RoomPlayer
   {
      PlayerId playerId = 0;
      SessionId sessionId = 0;
      
      std::string nickname;
      
      ObjectId pawnObjectId{};
      
      bool joined = false;
      bool loaded = false;
      
      bool death = false;  // 플레이어가 최종 죽음 상태인지 여부 (부활 불가능한 상태)
      
      // TODO: Room 로직에서 필요한 추가 정보 (예: 플레이어 상태, 위치, 이동 동기화 시간 등) 캐싱
   };

private:
   RoomId roomId_;
   TickSeq tickSeq_{0};
   
   ObjectManager objectManager_;                            // Room 내의 모든 오브젝트를 관리하는 ObjectManager (정본 컨테이너)
   
   std::unordered_map<PlayerId, RoomPlayer> roomPlayers_;   // Room Membership cache (플레이어 ID -> RoomPlayer 정보)
   
   RoomGameSystem roomGameSystem_{};
   const GameDataManager* gameDataManager_ = nullptr;   // non-owning
   GameConfig gameConfig_{};
   
   Random32 rng_{};
   TimerId closeTimerId_{0};
   
   RoomState roomState_{};
   float lastDeltaTime_ = 0.0f;   // 마지막 Tick에서의 Delta Time (초 단위)
    
};

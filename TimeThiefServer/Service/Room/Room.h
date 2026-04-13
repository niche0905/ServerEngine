#pragma once
#include <mutex>

#include "RoomState.h"
#include "Content/Object/ObjectId.h"
#include "Content/Gameplay/Combat/DamageTypes.h"
#include "Content/Object/ObjectManager.h"
#include "Service/Player/Player.h"
#include "Generated/ServerPacketHandler.h"
#include "Physics/Hitbox/HitResult.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"
#include "Systems/RoomGameSystem.h"

struct GameConfig;
class PlayerPawn;
class GameShard;
class GameDataManager;
class SessionManager;
struct DamageContext;
class Pawn;
class BaseObject;
class ObjectManager;
class Player;
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
public:
   using Vector3 = SE::Math::Vector3;
   
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
   
public:
   bool Init(GameShard* ownerShard, const GameDataManager& gameDataManager, const GameConfig& gameConfig);
   void SetPlayer(const std::vector<PlayerId>& playerIds);
   
public:
   bool Join(PlayerId playerId, SessionId sessionId);
   bool Leave(PlayerId playerId);
   
   bool UpdateSession(PlayerId playerId, SessionId newSessionId);
   
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
   bool HandlePickupItem(PlayerId playerId, const se::game::C_PickupItemReq& pkt);
   bool HandleSetSavePoint(PlayerId playerId, const se::game::C_SetSavePointReq& pkt);
   bool HandleJump(PlayerId playerId, const se::game::C_JumpReq& pkt);
   bool HandleJumpLand(PlayerId playerId, const se::game::C_JumpLand& pkt);
   bool HandleCrouch(PlayerId playerId, const se::game::C_CrouchReq& pkt);
   bool HandleWireAction(PlayerId playerId, const se::game::C_WireActionReq& pkt);
   bool HandleWireActionEnd(PlayerId playerId, const se::game::C_WireActionEnd& pkt);
   
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
   template <typename Func>
   void ForEachPawn(Func&& func)
   {
      for (ObjectId pawnId : pawnObjects_) {
         Pawn* pawn = objectManager_.FindAs<Pawn>(pawnId);
         if (pawn) {
            func(*pawn);
         }
      }
   }

public:
   bool Start();
   
   void UpdateTick(Milliseconds tickInterval);
   bool TraceHit(const SE::Physics::Ray& ray, ObjectId exceptId, SE::Physics::Hit::HitResult& outHit) const;   
   
   bool IsPlaying() const { return roomState_ == RoomState::Playing; }
   
public:
   TimerId ScheduleAt(TimePoint executeAt, Job job);
   TimerId ScheduleAfter(Duration delay, Job job);
   
public:
   std::shared_ptr<Room> GetRoomRef() { return shared_from_this(); }
   RoomId GetRoomId() const { return roomId_; }
   
   ObjectManager& GetObjectManager() { return objectManager_; }
   const ObjectManager& GetObjectManager() const { return objectManager_; }
   
   GameShard* GetOwnerShard() { return ownerShard_; }
   const GameShard* GetOwnerShard() const { return ownerShard_; }
   
   RoomGameSystem& GetRoomGameSystem() { return roomGameSystem_; }
   const RoomGameSystem& GetRoomGameSystem() const { return roomGameSystem_; }

   RoomState GetRoomState() const { return roomState_; }
   void SetRoomState(RoomState state) { roomState_ = state; }
   
public:
   bool HasPlayer(PlayerId playerId) const;
   SessionId GetSessionId(PlayerId playerId) const;
   ObjectId GetObjectId(PlayerId playerId) const;
   
private:
   PlayerPawn* CreatePreparedPlayerPawn(PlayerId playerId, const Vector3& spawnPos);
   
private:
   void Broadcast(std::shared_ptr<SendBuffer> sendBuffer, PlayerId exceptPlayerId = 0);
   bool SendToPlayer(PlayerId playerId, SendBufferRef sendBuffer);
   
public:
   void BroadcastGameStart();
   void BroadcastDeath(ObjectId objectId);
   void NotifyHealthChange(PlayerId id, int newHealth, int deltaHealth);
   // void BroadcastHit();
   void BroadcastKillPlayer(ObjectId killerId, ObjectId victimId);
   
public:
   void HandleDamageResult(Pawn* attacker, Actor* victim, const SE::Physics::Hit::HitResult& hitResult, const DamageContext& ctx, const DamageResult& damageResult);
   void HandlePawnDeath(ObjectId pawnId, const DamageResult& damageResult);
   
public:
   void OnZoneChanged(uint32 phase, const ZoneCircle& newZone, float waitDuration, float shrinkDuration);
   
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
      
      ObjectId pawnObjectId{};
      
      bool joined = false;
      bool loaded = false;
      
      // TODO: Room 로직에서 필요한 추가 정보 (예: 플레이어 상태, 위치, 이동 동기화 시간 등) 캐싱
   };

private:
   RoomId roomId_;
   
   ObjectManager objectManager_;                            // Room 내의 모든 오브젝트를 관리하는 ObjectManager (정본 컨테이너)
   
   std::unordered_map<PlayerId, RoomPlayer> roomPlayers_;   // Room Membership cache (플레이어 ID -> RoomPlayer 정보)
   
   // TODO: ObjectHandle로 가지고 있는 것도 괜찮을 듯...?
   std::unordered_set<ObjectId> pawnObjects_;               // Pawn들
   std::vector<ObjectId> npcTickList_;                      // 매 틱마다 업데이트가 필요한 NPC들의 ID 리스트
   RoomGameSystem roomGameSystem_{};
   
   RoomState roomState_{};
    
};

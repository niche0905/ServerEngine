#pragma once
#include <mutex>
#include "Content/Object/ObjectManager.h"
#include "Service/Player/Player.h"
#include "Generated/ServerPacketHandler.h"

class BaseObject;
class ObjectManager;
class Player;

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
   static std::shared_ptr<Room> Create(RoomId roomId)
   {
      auto room = std::make_shared<Room>(roomId);
      room->PostCreate();
      return room;
   }

public:
   explicit Room(RoomId roomId);
   ~Room();
   
   void PostCreate();
   
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
   void UpdateTick();
   
public:
   std::shared_ptr<Room> GetRoomRef() { return shared_from_this(); }
   RoomId GetRoomId() const { return roomId_; }
   
   bool HasPlayer(PlayerId playerId) const;
   SessionId GetSessionId(PlayerId playerId) const;
   ObjectId GetObjectId(PlayerId playerId) const;
   
private:
   void Broadcast(std::shared_ptr<SendBuffer> sendBuffer, PlayerId exceptPlayerId = 0);
   
private:
   void IndexObject_OnAdd(BaseObject* object);
   void IndexObject_OnRemove(ObjectId objectId);
   
private:
   struct RoomPlayer
   {
      PlayerId playerId = 0;
      SessionId sessionId = 0;
      
      ObjectId pawnObjectId{};
      bool loadingComplete = false;
      
      // TODO: Room 로직에서 필요한 추가 정보 (예: 플레이어 상태, 위치, 이동 동기화 시간 등) 캐싱
   };

   // TEMP: 샤딩을 통한 thread safety가 보장되기 전 Lock 구조
private:
   mutable std::mutex mutex_;
   
private:
   RoomId roomId_;
   
   ObjectManager objectManager_;                            // Room 내의 모든 오브젝트를 관리하는 ObjectManager (정본 컨테이너)
   
   std::unordered_map<PlayerId, RoomPlayer> roomPlayers_;   // Room Membership cache (플레이어 ID -> RoomPlayer 정보)
   
   std::unordered_set<ObjectId> pawnObjects_;               // Pawn들
   std::vector<ObjectId> npcTickList_;                      // 매 틱마다 업데이트가 필요한 NPC들의 ID 리스트
    
};

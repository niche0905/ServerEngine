#pragma once
#include "SpawnTypes.h"
#include <unordered_map>

class ObjectManager;
struct SE::Math::Vector3;

/*-----------------
   SpawnService
-----------------*/
//
// SpawnService는 Lifetime Despawn을 전담합니다
//

class SpawnService
{
public:
   using RoomId = uint32;
   
public:
   explicit SpawnService(RoomId roomId)
      : roomId_(roomId)
   {
   }
   
   // 만료 상대 시간 기준 등록
   void RegisterLifetimeMs(ObjectId obj, uint64 nowMs, uint32 lifetimeMs);
   
   // 절대 시간 기준 등록
   void RegisterLifetime(ObjectId obj, uint64 expireAtMs);
   
   // OnDestroy(혹 OnPreDestroy) 시 호출
   void Unregister(ObjectId obj);
   
   // Room tick에서 호출
   void Update(ObjectManager& om, uint64 nowMs);
   
   // Room Reset 시 호출
   void Clear();
   
private:
   void UpdateDespawn(ObjectManager& om, uint64 nowMs);
   
private:
   RoomId roomId_;
   std::unordered_map<ObjectId, SpawnMeta> meta_;
   
};

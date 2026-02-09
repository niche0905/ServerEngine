#include "pch.h"
#include "SpawnService.h"
#include "ISpawnFactory.h"
#include "Content/Object/ObjectManager.h"
#include "Utils/Random/WeightedRandom.h"

/*-----------------
   Local Helper
-----------------*/

namespace 
{
   inline bool IsValidObject(ObjectManager& om, ObjectId obj)
   {
      return om.IsValid(obj);
   }
   
   inline void DespawnObject(ObjectManager& om, ObjectId obj, DespawnReason reason)
   {
      om.RequestDestroy(obj);
      (void)reason;
   }
}

/*-----------------
   SpawnService
-----------------*/

void SpawnService::RegisterLifetimeMs(ObjectId obj, uint64 nowMs, uint32 lifetimeMs)
{
   if (obj == ObjectId{}) return;
   
   if (lifetimeMs == 0) {  // 만료되지 않을 경우라면 등록하지 않음
      meta_.erase(obj);
      return;
   }
   
   const uint64 expireAt = nowMs + static_cast<uint64>(lifetimeMs);
   RegisterLifetime(obj, expireAt);
}

void SpawnService::RegisterLifetime(ObjectId obj, uint64 expireAtMs)
{
   if (obj == ObjectId{}) return;
   
   if (expireAtMs == 0) {  // 만료되지 않을 경우라면 등록하지 않음
      meta_.erase(obj);
      return;
   }
   
   meta_[obj] = SpawnMeta{ std::max(expireAtMs, meta_[obj].expireAtMs) };
   // TODO: 위 코드는 문제가 없나?
}

void SpawnService::Unregister(ObjectId obj)
{
   if (obj == ObjectId{}) return;
   
   meta_.erase(obj);
}

void SpawnService::Update(ObjectManager& om, uint64 nowMs)
{
   UpdateDespawn(om, nowMs);
}

void SpawnService::Clear()
{
   meta_.clear();
}

void SpawnService::UpdateDespawn(ObjectManager& om, uint64 nowMs)
{
   if (meta_.empty()) return;
   
   for (auto it = meta_.begin(); it != meta_.end(); )
   {
      const ObjectId obj = it->first;
      const uint64 expireAt = it->second.expireAtMs;
      
      if (not IsValidObject(om, obj) {
         it = meta_.erase(it);
         continue;
      }
      
      if (expireAt != 0 and nowMs >= expireAt) {
         DespawnObject(om, obj, DespawnReason::LifetimeExpired);
         it = meta_.erase(it);
         continue;
      }

      ++it;
   }
}

#pragma once
#include "SpawnTypes.h"
#include "RespawnTypes.h"
#include <unordered_map>

class ObjectManager;
struct SE::Math::Vector3;
class ISpawnFactory;

/*-----------------
   SpawnService
-----------------*/
//
// SpawnService는 스폰 관련 기능을 제공하는 서비스입니다.
//

class SpawnService
{
public:
   using Vector3 = SE::Math::Vector3;
   
public:
   explicit SpawnService(ISpawnFactory& factory)
      : factory_(factory)
   {
   }
   
   void AddSpawnPoint(const SpawnPoint& sp)
   {
      spawnPoints_[sp.id] = sp;
   }
   
   bool HasSpawnPoint(int32 id) const
   {
      return spawnPoints_.find(id) != spawnPoints_.end();
   }
   
   int32 InitialSpawn(ObjectManager& om, uint64 nowMs);
   
   void Update(ObjectManager& om, uint64 nowMs);
   
   void NotifyDead(int32 spawnPointId, ObjectId obj, uint64 nowMs);
   
   int32 DespawnAll(ObjectManager& om, DespawnReason reason, uint64 nowMs);
   
private:
   bool TrySpawnOne(ObjectManager& om, const SpawnPoint& sp, uint64 nowMs);
   void UpdateRespawn(ObjectManager& om, uint64 nowMs);
   void UpdateDespawn(ObjectManager& om, uint64 nowMs);
   
private:
   ISpawnFactory& factory_;
   std::unordered_map<int32, SpawnPoint> spawnPoints_;      // 스폰 포인트 ID별 정의
   
   // TODO: 정적 데이터를 가리키는 것은 추후 생각
    
};

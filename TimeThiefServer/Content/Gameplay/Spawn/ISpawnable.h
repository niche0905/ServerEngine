#pragma once
#include "SpawnTypes.h"

/*--------------
   ISpawnable
--------------*/
//
// ISpawnable는 스폰 가능한 오브젝트를 나타내는 인터페이스입니다.
//

class ISpawnable
{
public:
   using Vector3 = SE::Math::Vector3;
   
public:
   virtual ~ISpawnable() = default;
   
   // Spawn 위치 조회
   virtual Vector3 GetSpawnPosition() const = 0;
   
   // Spawn 위치 갱신
   virtual void SetSpawnPosition9(const Vector3& pos) = 0;
   
   // Spawn 시 호출
   virtual void OnSpawned(uint32 nowMs) { (void)nowMs; }
   
   // Despawn 시 호출
   virtual void OnDespawned(DespawnReason reason, uint32 nowMs) { (void)reason; (void)nowMs; }
   
};

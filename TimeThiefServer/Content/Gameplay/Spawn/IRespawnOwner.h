#pragma once
#include "Content/Object/ObjectManager.h"

class ObjectManager;
struct SE::Math::Vector3;

/*-----------------
   IRespawnOwner
-----------------*/
//
// IRespawnOwner는 리스폰 기능을 소유한 오브젝트가 구현해야 하는 인터페이스입니다.
//

class IRespawnOwner
{
public:
   using Vector3 = SE::Math::Vector3;
   
public:
   virtual ~IRespawnOwner() = default;
   
   virtual Vector3 ResolveRespawnPosition(ObjectManager& om) = 0;
   
   // 리스폰 직전에 호출됩니다.
   virtual void OnPreRespawn(ObjectManager& om) = 0;
   
   // 리스폰 직후에 호출됩니다.
   virtual void OnPostRespawn(ObjectManager& om) = 0;
   
   // 월드 반영
   virtual void ApplyRespawnToWorld(ObjectManager& om, const Vector3& pos) = 0;
   
   // 리스폰 무적 부여
   virtual void GrantSpawnInvulnerability(ObjectManager& om, uint32 durationMs) = 0;
};

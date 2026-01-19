#include "pch.h"
#include "HitboxInstance.h"

/*------------------
   HitboxInstance
------------------*/

namespace SE::Physics::Hit
{
   void HitboxInstance::Bind(const HitboxAsset* asset)
   {
      asset_ = asset;
   }

   void HitboxInstance::Update(const Vector3& rootPos, float yawRadians)
   {
      // TODO: 채워라
   }

   const AABBCollider& HitboxInstance::GetWorldAABB() const
   {
      return worldAABB_;
   }
}

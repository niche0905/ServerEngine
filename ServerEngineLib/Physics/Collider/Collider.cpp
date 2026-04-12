#include "pch.h"
#include "Collider.h"
#include "CollisionResult.h"
#include "Physics/Narrowphase/IntersectTable.h"

/*------------
   Collider
------------*/


namespace SE::Physics
{
   bool Collider::Intersect(const Collider& other, CollisionResult& out) const
   {
      if (other.GetType() == ColliderType::Compound) {
         // Compound vs Other 충돌 판정은 반대 순서로 처리
         CollisionResult tempResult;
         if (not other.Intersect(*this, tempResult))
            return false;
         
         tempResult.normal = -tempResult.normal;
         out = tempResult;
         return true;
      }
      
      using namespace SE::Physics::NarrowPhase;
      
      const auto fn = GetIntersectFn(GetType(), other.GetType());
      return fn(*this, other, out);
   }
}

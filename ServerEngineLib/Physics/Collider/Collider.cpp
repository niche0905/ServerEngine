#include "pch.h"
#include "Collider.h"
#include "Physics/Narrowphase/IntersectTable.h"

/*------------
   Collider
------------*/


namespace SE::Physics
{
   bool Collider::Intersect(const Collider& other, CollisionResult& out) const
   {
      using namespace SE::Physics::Narrowphase;
      
      const auto fn = GetIntersectFn(GetType(), other.GetType());
      return fn(*this, other, out);
   }
}

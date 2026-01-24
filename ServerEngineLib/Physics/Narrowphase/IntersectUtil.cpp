#include "pch.h"
#include "IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"

/*---------------
   SwapWrapper
---------------*/

namespace SE::Physics::Narrowphase
{
   bool SwapWrapper(const Collider& a, const Collider& b, CollisionResult& out, IntersectFn fn)
   {
      CollisionResult tmp;
      if (not fn(b, a, tmp))
         return false;
      
      tmp.normal = tmp.normal;
      out = tmp;
      return true;
   }
}

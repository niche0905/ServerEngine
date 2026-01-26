#include "pch.h"
#include "IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"

/*---------------
   SwapWrapper
---------------*/

namespace SE::Physics::Narrowphase
{
   bool SwapWrapper(const Collider& a, const Collider& b, CollisionResult& out, IntersectFn fnBA)
   {
      CollisionResult tmp;
      if (not fnBA(b, a, tmp))
         return false;
      
      tmp.normal = -tmp.normal;
      out = tmp;
      return true;
   }
   
   Vector3 ClampPointAABB(const Vector3& p, const Vector3& mn, const Vector3& mx)
   {
      return Vector3(
         SE::Math::Clamp(p.x, mn.x, mx.x),
         SE::Math::Clamp(p.y, mn.y, mx.y),
         SE::Math::Clamp(p.z, mn.z, mx.z)
      );
   }
   
   bool IntersectSegmentAABB(const Vector3& A, const Vector3& B, const Vector3& mn, const Vector3& mx, float& outTEnter)
   {
      const Vector3 d = B - A;
      float tMin = 0.0f;
      float tMax = 1.0f;
      
      auto slab = [&](float a, float da, float minB, float maxB) -> bool
      {
         if (SE::Math::Abs(da) <= 1e-12f) {
            return (a >= minB and a <= maxB);
         }
         const float invD = 1.0f / da;
         float t1 = (minB - a) * invD;
         float t2 = (maxB - a) * invD;
         if (t1 > t2) std::swap(t1, t2);
         
         if (t1 > tMin) tMin = t1;
         if (t2 < tMax) tMax = t2;
         return (tMin <= tMax);
      };
      
      if (!slab(A.x, d.x, mn.x, mx.x)) return false;
      if (!slab(A.y, d.y, mn.y, mx.y)) return false;
      if (!slab(A.z, d.z, mn.z, mx.z)) return false;
      
      outTEnter = tMin;
      return true;
   }
   
   void ClosestSegmentAABB_AltProj(const Vector3& A, const Vector3& B, const Vector3& mn, const Vector3& mx, float& ioT, Vector3& outP, Vector3& outQ)
   {
      const Vector3 AB = B - A;
      const float abLenSq = AB.LengthSq();
      
      auto projT = [&](const Vector3& X) -> float
      {
         if (abLenSq <= 1e-12f) return 0.0f;
         return SE::Math::Clamp((X - A).Dot(AB) / abLenSq, 0.0f, 1.0f);
      };
      
      float t = SE::Math::Clamp(ioT, 0.0f, 1.0f);
      
      for (int iter = 0; iter < 2; ++iter) {
         outP = A + AB * t;
         outQ = ClampPointAABB(outP, mn, mx);
         t = projT(outQ);
      }
      
      outP = A + AB * t;
      outQ = ClampPointAABB(outP, mn, mx);
      ioT = t;
   }
}

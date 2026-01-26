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

   void ResolveInsideAABBPointNormal(const Vector3& c, const Vector3& boxMin, const Vector3& boxMax, Vector3& outNormal,
      Vector3& outPoint, float& outFaceDist)
   {
      const float dxMin = SE::Math::Abs(c.x - boxMin.x);
      const float dxMax = SE::Math::Abs(boxMax.x - c.x);
      const float dyMin = SE::Math::Abs(c.y - boxMin.y);
      const float dyMax = SE::Math::Abs(boxMax.y - c.y);
      const float dzMin = SE::Math::Abs(c.z - boxMin.z);
      const float dzMax = SE::Math::Abs(boxMax.z - c.z);
      
      const float dx = SE::Math::Min(dxMin, dxMax);
      const float dy = SE::Math::Min(dyMin, dyMax);
      const float dz = SE::Math::Min(dzMin, dzMax);
      
      outPoint = c;
      outNormal = Vector3{0, 0, 0};
      outFaceDist = 0.0f;
      
      if (dx <= dy and dx <= dz) {
         // X축 면이 가장 가까움
         if (dxMin <= dxMax) {
            outNormal = Vector3{-1, 0, 0};
            outPoint.x = boxMin.x;
            outFaceDist = dxMin;
         }
         else {
            outNormal = Vector3{1, 0, 0};
            outPoint.x = boxMax.x;
            outFaceDist = dxMax;
         }
      }
      else if (dy <= dx and dy <= dz) {
         // Y축 면이 가장 가까움
         if (dyMin <= dyMax) {
            outNormal = Vector3{0, -1, 0};
            outPoint.y = boxMin.y;
            outFaceDist = dyMin;
         }
         else {
            outNormal = Vector3{0, 1, 0};
            outPoint.y = boxMax.y;
            outFaceDist = dyMax;
         }
      }
      else {
         // Z축 면이 가장 가까움
         if (dzMin <= dzMax) {
            outNormal = Vector3{0, 0, -1};
            outPoint.z = boxMin.z;
            outFaceDist = dzMin;
         }
         else {
            outNormal = Vector3{0, 0, 1};
            outPoint.z = boxMax.z;
            outFaceDist = dzMax;
         }
      }
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

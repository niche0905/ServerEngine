#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/AABBCollider.h"
#include "Physics/Collider/CapsuleCollider.h"


namespace SE::Physics::Narrowphase
{
   using Vector3 = SE::Math::Vector3;
   
   bool Intersect_AABB_Capsule(const Collider& a, const Collider& b, CollisionResult& out)
   {
      const auto& A = static_cast<const AABBCollider&>(a);
      const auto& C = static_cast<const CapsuleCollider&>(b);
      
      const Vector3 boxMin = A.GetMin();
      const Vector3 boxMax = A.GetMax();
      
      const Vector3 A0 = C.GetPointA();
      const Vector3 B0 = C.GetPointB();
      const float r = C.GetRadius();
      const float rSq = r * r;
      
      const Vector3 inflMin = boxMin - Vector3(r, r, r);
      const Vector3 inflMax = boxMax + Vector3(r, r, r);
      
      float tEnter = 0.0f;
      if (not IntersectSegmentAABB(A0, B0, inflMin, inflMax, tEnter))
         return false;
      
      Vector3 P, Q;  // P: 캡슐 선분 상의 최근접 점, Q: AABB 상의 최근접 점
      float t = tEnter;
      ClosestSegmentAABB_AltProj(A0, B0, boxMin, boxMax, t, P, Q);
      
      const Vector3 v = Q - P;
      const float distSq = v.LengthSq();
      
      if (distSq > rSq)
         return false;
      
      out.hit = true;
      
      if (distSq > 1e-12f) {
         const float dist = std::sqrt(distSq);
         const Vector3 n = v * (1.0f / dist);
         out.normal = n;
         out.penetration = r - dist;
         out.point = Q;
         return true;
      } 
      
      const Vector3 c = P;
      const float dx = SE::Math::Min(SE::Math::Abs(c.x - boxMin.x), SE::Math::Abs(boxMax.x - c.x));
      const float dy = SE::Math::Min(SE::Math::Abs(c.y - boxMin.y), SE::Math::Abs(boxMax.y - c.y));
      const float dz = SE::Math::Min(SE::Math::Abs(c.z - boxMin.z), SE::Math::Abs(boxMax.z - c.z));
      
      Vector3 n(0, 0, 0);
      Vector3 pt = c;
      
      if (dx <= dy and dx <= dz) {
         if (SE::Math::Abs(c.x - boxMin.x) <= SE::Math::Abs(boxMax.x - c.x)) {
            n = Vector3(-1, 0, 0);
            pt.x = boxMin.x;
         } else {
            n = Vector3(1, 0, 0);
            pt.x = boxMax.x;
         }
         out.penetration = r + dx;
      } else if (dy <= dx and dy <= dz) {
         if (SE::Math::Abs(c.y - boxMin.y) <= SE::Math::Abs(boxMax.y - c.y)) {
            n = Vector3(0, -1, 0);
            pt.y = boxMin.y;
         } else {
            n = Vector3(0, 1, 0);
            pt.y = boxMax.y;
         }
         out.penetration = r + dy;
      } else {
         if (SE::Math::Abs(c.z - boxMin.z) <= SE::Math::Abs(boxMax.z - c.z)) {
            n = Vector3(0, 0, -1);
            pt.z = boxMin.z;
         } else {
            n = Vector3(0, 0, 1);
            pt.z = boxMax.z;
         }
         out.penetration = r + dz;
      }
      
      out.normal = n;
      out.point = pt;
      
      return true;
   }

   bool Intersect_Capsule_AABB(const Collider& a, const Collider& b, CollisionResult& out)
   {
      // a가 캡슐, b가 AABB인 경우는 AABB와 캡슐의 교차 검사로 위임
      return SwapWrapper(a, b, out, &Intersect_AABB_Capsule);
   }
}

#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/SphereCollider.h"

namespace SE::Physics::NarrowPhase
{
   bool Intersect_Sphere_Sphere(const Collider& a, const Collider& b, CollisionResult& out)
   {
      const auto& A = static_cast<const SphereCollider&>(a);
      const auto& B = static_cast<const SphereCollider&>(b);
      
      const Vector3 ca = A.GetCenter();
      const Vector3 cb = B.GetCenter();
      const float ra = A.GetRadius();
      const float rb = B.GetRadius();
      
      const Vector3 d = ca - cb;
      const float distSq = d.LengthSq();
      const float radiusSum = ra + rb;
      const float radiusSumSq = radiusSum * radiusSum;
      
      if (distSq > radiusSumSq) {
         return false; // 충돌 없음
      }
      
      out.hit = true;
      
      if (distSq <= 1e-12f) {
         out.normal = Vector3{0.0f, 1.0f, 0.0f};
         out.penetration = radiusSum;
         out.point = (ca + cb) * 0.5f;
         
         return true;
      }
      
      const float dist = std::sqrt(distSq);
      const Vector3 n = d * (1.0f / dist);
      
      out.normal = n;
      out.penetration = radiusSum - dist;
      
      const Vector3 pa = ca - n * ra;
      const Vector3 pb = cb + n * rb;
      out.point = (pa + pb) * 0.5f;
      
      return true;
   }
    
}

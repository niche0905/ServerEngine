#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/AABBCollider.h"
#include "Physics/Collider/SphereCollider.h"


namespace SE::Physics::Narrowphase
{
   using Vector3 = SE::Math::Vector3;
   
   bool Intersect_AABB_Sphere(const Collider& a, const Collider& b, CollisionResult& out)
   {
      const auto& A = static_cast<const AABBCollider&>(a);
      const auto& S = static_cast<const SphereCollider&>(b);
      
      const Vector3& mn = A.GetMin();
      const Vector3& mx = A.GetMax();
      
      const Vector3& c = S.GetCenter();
      const float r = S.GetRadius();
      const float rSq = r * r;
      
      // AABB의 가장 가까운 점을 구함
      const Vector3 p = {
         SE::Math::Clamp(c.x, mn.x, mx.x),
         SE::Math::Clamp(c.y, mn.y, mx.y),
         SE::Math::Clamp(c.z, mn.z, mx.z)
      };
      
      const Vector3 v = c - p;
      const float distSq = v.LengthSq();
      if (distSq > rSq)
         return false;
      
      out.hit = true;
      
      Vector3 n;
      float dist = 0.0f;
      
      if (distSq > 1e-12f) {
         dist = std::sqrt(distSq);
         n = (p - c) * (1.0f / dist);
         out.penetration = r - dist;
         out.point = p;
         out.normal = n;
         
         return true;
      }
      
      // distSq == 0이면 구의 중심이 AABB 내부에 있음
      // 가장 가까운 면을 찾아서 법선 벡터를 결정
      Vector3 nFace, pFace;
      float faceDist = 0.0f;
      
      ResolveInsideAABBPointNormal(c, mn, mx, nFace, pFace, faceDist);
      
      out.normal = nFace;
      out.point = pFace;
      out.penetration = r + faceDist;
      
      return true;
   }

   bool Intersect_Sphere_AABB(const Collider& a, const Collider& b, CollisionResult& out)
   {
      return SwapWrapper(a, b, out, &Intersect_AABB_Sphere);
   }
}

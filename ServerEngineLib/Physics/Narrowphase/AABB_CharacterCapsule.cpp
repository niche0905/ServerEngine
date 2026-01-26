#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/AABBCollider.h"
#include "Physics/Collider/CharacterCapsuleCollider.h"


namespace SE::Physics::Narrowphase
{
   using Vector3 = SE::Math::Vector3;
   
   bool Intersect_AABB_CharacterCapsule(const Collider& a, const Collider& b, CollisionResult& out)
   {
      const auto& box = static_cast<const AABBCollider&>(a);
      const auto& cap = static_cast<const CharacterCapsuleCollider&>(b);
      
      const Vector3 boxMin = box.GetMin();
      const Vector3 boxMax = box.GetMax();
      
      const Vector3 A0 = cap.GetPointA();
      const Vector3 B0 = cap.GetPointB();
      const float r = cap.GetRadius();
      const float rSq = r * r;
      
      const Vector3 inflMin = boxMin - Vector3(r, r, r);
      const Vector3 inflMax = boxMax + Vector3(r, r, r);
      
      float tEnter = 0.0f;
      if (not IntersectSegmentAABB(A0, B0, inflMin, inflMax, tEnter)) {
         return false;
      }
      
      Vector3 P, Q;  // P: 캡슐 중심선 상의 최근접 점, Q: AABB 상의 최근접 점
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
      
      // distSq == 0 segment의 point가 AABB 내부에 있는 경우
      
      Vector3 nFace, pFace;
      float faceDist = 0.0f;
      
      ResolveInsideAABBPointNormal(P, boxMin, boxMax, nFace, pFace, faceDist);
      
      out.normal = nFace;
      out.point = pFace;
      out.penetration = r + faceDist;
      
      return true;
   }

   bool Intersect_CharacterCapsule_AABB(const Collider& a, const Collider& b, CollisionResult& out)
   {
      // a가 캡슐, b가 AABB인 경우는 AABB와 캡슐의 교차 검사로 위임
      return SwapWrapper(a, b, out, &Intersect_AABB_CharacterCapsule);
   }
}

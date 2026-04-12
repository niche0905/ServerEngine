#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/AABBCollider.h"

namespace SE::Physics::NarrowPhase
{
   bool Intersect_AABB_AABB(const Collider& a, const Collider& b, CollisionResult& out)
   {
      using Vector3 = SE::Math::Vector3;
      
      const auto& A = static_cast<const AABBCollider&>(a);
      const auto& B = static_cast<const AABBCollider&>(b);
      
      const Vector3& aMin = A.GetMin();
      const Vector3& aMax = A.GetMax();
      const Vector3& bMin = B.GetMin();
      const Vector3& bMax = B.GetMax();
      
      // 축별로 겹침 검사
      // if (not A.Overlaps(B))  <- 이걸로 대체 가능하지 않나?
      //    return false;
      if (aMax.x < bMin.x or aMin.x > bMax.x) return false; // X축 겹침 없음
      if (aMax.y < bMin.y or aMin.y > bMax.y) return false; // Y축 겹침 없음
      if (aMax.z < bMin.z or aMin.z > bMax.z) return false; // Z축 겹침 없음
      
      const float overlapX = SE::Math::Min(aMax.x, bMax.x) - SE::Math::Max(aMin.x, bMin.x);
      const float overlapY = SE::Math::Min(aMax.y, bMax.y) - SE::Math::Max(aMin.y, bMin.y);
      const float overlapZ = SE::Math::Min(aMax.z, bMax.z) - SE::Math::Max(aMin.z, bMin.z);
      
      float pen = overlapX;
      Vector3 n{1, 0, 0};
      
      const Vector3& aC = A.GetCenter();  // 위험할 수도 있다 const Vector로 받는거 추천
      const Vector3& bC = B.GetCenter();
      const Vector3 d = aC - bC;
      
      // X축
      n = (d.x >= 0.0f) ? Vector3{1, 0, 0} : Vector3{-1, 0, 0};
      pen = overlapX;
      Vector3 bestN = n;
      
      // Y축
      {
         const float p = overlapY;
         const Vector3 nn = (d.y >= 0.0f) ? Vector3{0, 1, 0} : Vector3{0, -1, 0};
         if (p < pen) { pen = p; bestN = nn; }
      }
      
      // Z축
      {
         const float p = overlapZ;
         const Vector3 nn = (d.z >= 0.0f) ? Vector3{0, 0, 1} : Vector3{0, 0, -1};
         if (p < pen) { pen = p; bestN = nn; }
      }
      
      out.hit = true;
      out.normal = bestN;
      out.penetration = pen;
      
      const Vector3 iMin{
         SE::Math::Max(aMin.x, bMin.x),
         SE::Math::Max(aMin.y, bMin.y),
         SE::Math::Max(aMin.z, bMin.z)
      };
      
      const Vector3 iMax{
         SE::Math::Min(aMax.x, bMax.x),
         SE::Math::Min(aMax.y, bMax.y),
         SE::Math::Min(aMax.z, bMax.z)
      };
      
      out.point = (iMin + iMax) * 0.5f;
      
      return true;
   }
    
}

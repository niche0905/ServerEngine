#include "pch.h"
#include "AABBCollider.h"
#include "CollisionResult.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"

/*----------------
   AABBCollider
----------------*/

namespace SE::Physics
{
   AABBCollider::AABBCollider(const Vector3& minPoint, const Vector3& maxPoint)
   {
      SetMinMax(minPoint, maxPoint);
   }

   ColliderType AABBCollider::GetType() const
   {
      return ColliderType::AABB;
   }

   Collider* AABBCollider::Clone() const
   {
      return new AABBCollider(*this);
   }

   bool AABBCollider::Intersect(const Collider& other, CollisionResult& out) const
   {
      switch (other.GetType())
      {
      case ColliderType::AABB:
         {
            const auto& b = static_cast<const AABBCollider&>(other);
            const bool hit = Overlaps(b);
            
            // TODO: 정확한 CollisionResult가 필요하면 여기에서 채워넣어야 한다
            
            out.hit = hit;
            return hit;
         }
         
         // TODO: 충돌체 들 구현 되면 여기에 추가
      default:
         out.hit = false;
         return false;
      }
   }

   void AABBCollider::SetMinMax(const Vector3& minPoint, const Vector3& maxPoint)
   {
      min_.x = SE::Math::Min(minPoint.x, maxPoint.x);
      min_.y = SE::Math::Min(minPoint.y, maxPoint.y);
      min_.z = SE::Math::Min(minPoint.z, maxPoint.z);
      
      max_.x = SE::Math::Max(minPoint.x, maxPoint.x);
      max_.y = SE::Math::Max(minPoint.y, maxPoint.y);
      max_.z = SE::Math::Max(minPoint.z, maxPoint.z);
      
      RecalcCache();
   }

   bool AABBCollider::Contains(const Vector3& point) const
   {
      return (point.x >= min_.x && point.x <= max_.x) and 
         (point.y >= min_.y && point.y <= max_.y) and
         (point.z >= min_.z && point.z <= max_.z);
   }

   bool AABBCollider::Overlaps(const AABBCollider& other) const
   {
      if (max_.x < other.min_.x or min_.x > other.max_.x) return false;
      if (max_.y < other.min_.y or min_.y > other.max_.y) return false;
      if (max_.z < other.min_.z or min_.z > other.max_.z) return false;
      
      return true;
   }

   void AABBCollider::Expand(float margin)
   {
      min_.x -= margin; min_.y -= margin; min_.z -= margin;
      max_.x += margin; max_.y += margin; max_.z += margin;
      
      RecalcCache();
   }

   AABBCollider AABBCollider::Union(const AABBCollider& a, const AABBCollider& b)
   {
      AABBCollider result;
      Vector3 mn{
         SE::Math::Min(a.min_.x, b.min_.x),
         SE::Math::Min(a.min_.y, b.min_.y),
         SE::Math::Min(a.min_.z, b.min_.z)
      };
      Vector3 mx{
         SE::Math::Max(a.max_.x, b.max_.x),
         SE::Math::Max(a.max_.y, b.max_.y),
         SE::Math::Max(a.max_.z, b.max_.z)
      };
      result.SetMinMax(mn, mx);
      
      return result;
   }

   const AABBCollider& AABBCollider::GetWorldAABB() const
   {
      return *this;
   }

   bool AABBCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      const Vector3& mn = GetMin();
      const Vector3& mx = GetMax();
      
      float tMin = ray.tMin;
      float tMax = ray.tMax;
      
      Vector3 enterNormal = Vector3(0.0f, 0.0f, 0.0f);
      Vector3 exitNormal = Vector3(0.0f, 0.0f, 0.0f);
      
      auto slab = [&](float origin, float dir, float minB, float maxB, const Vector3& nEnter, const Vector3& nExit) -> bool
      {
         // Ray가 이 축에 평행한 경우
         if (std::fabs(dir) <= 1e-12f) {
            return (origin >= minB and origin <= maxB);
         }
         
         const float invD = 1.0f / dir;
         float t1 = (minB - origin) * invD;
         float t2 = (maxB - origin) * invD;
         
         Vector3 n1 = nEnter;
         Vector3 n2 = nExit;
         
         if (t1 > t2) {
            std::swap(t1, t2);
            std::swap(n1, n2);
         }
         
         // 구간 교집합
         if (t1 > tMin) {
            tMin = t1;
            enterNormal = n1;   // 가장 늦게 들어오는 면이 충돌한 것
         }
         if (t2 < tMax) {
            tMax = t2;
            exitNormal = n2;
         }
         
         // 교집합이 없으면 false
         return (tMin <= tMax);
      };
      
      // X slab
      if (not slab(ray.origin.x, ray.direction.x, mn.x, mx.x, Vector3(-1,0,0), Vector3(1,0,0)))
         return false;
      
      // Y slab
      if (not slab(ray.origin.y, ray.direction.y, mn.y, mx.y, Vector3(0,-1,0), Vector3(0,1,0)))
         return false;
      
      // Z slab
      if (not slab(ray.origin.z, ray.direction.z, mn.z, mx.z, Vector3(0,0,-1), Vector3(0,0,1)))
         return false;
      
      // 충돌 발생
      float tHit = tMin;
      Vector3 nHit = enterNormal;
      
      
      if (tHit < ray.tMin) {  // 내부 시작이면 exit를 사용
         tHit = tMax;
         nHit = exitNormal;
      }

      if (tHit < ray.tMin or tHit > ray.tMax)
         return false;
   
      out.hit = true;
      out.t = tHit;
      out.point = ray.At(tHit);
      out.normal = nHit;
      out.collider = this;
      
      return true;
   }

   void AABBCollider::RecalcCache()
   {
      center_ = (min_ + max_) * 0.5f;
      extent_ = (max_ - min_) * 0.5f;
   }
}

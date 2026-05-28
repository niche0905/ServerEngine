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

   void AABBCollider::UpdateWorld(const Math::Vector3& position, float yaw)
   {
      worldCenter_ = position + localCenter_;
      worldExtent_ = localExtent_;
      
      min_ = worldCenter_ - worldExtent_;
      max_ = worldCenter_ + worldExtent_;
   }

   void AABBCollider::SetMinMax(const Vector3& minPoint, const Vector3& maxPoint)
   {
      const Vector3 mn{
         SE::Math::Min(minPoint.x, maxPoint.x),
         SE::Math::Min(minPoint.y, maxPoint.y),
         SE::Math::Min(minPoint.z, maxPoint.z)
      };
      
      const Vector3 mx{
         SE::Math::Max(minPoint.x, maxPoint.x),
         SE::Math::Max(minPoint.y, maxPoint.y),
         SE::Math::Max(minPoint.z, maxPoint.z)
      };
      
      localCenter_ = (mn + mx) * 0.5f;
      localExtent_ = (mx - mn) * 0.5f;
      
      UpdateWorld(Vector3{0.0f, 0.0f, 0.0f}, 0.0f);
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

   bool AABBCollider::ContainsPoint(const Math::Vector3& point) const
   {
      return Contains(point);
   }

   bool AABBCollider::ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest,
      Math::Vector3& outNormal) const
   {
      constexpr float Epsilon = 1e-6f;

      // 1. 외부/경계 포함 closest point 계산
      Math::Vector3 closest{
         std::clamp(point.x, min_.x, max_.x),
         std::clamp(point.y, min_.y, max_.y),
         std::clamp(point.z, min_.z, max_.z)
      };

      // 2. 외부에 있는 점이면 clamp 결과가 표면 closest
      if (!Contains(point))
      {
         Math::Vector3 normal = point - closest;

         const float lenSq = normal.LengthSq();
         if (lenSq > Epsilon)
         {
            outClosest = closest;
            outNormal = normal.Normalized();
            return true;
         }

         return false;
      }

      // 3. 내부에 있는 점이면 가장 가까운 면 선택
      const float distMinX = std::abs(point.x - min_.x);
      const float distMaxX = std::abs(max_.x - point.x);
      const float distMinY = std::abs(point.y - min_.y);
      const float distMaxY = std::abs(max_.y - point.y);
      const float distMinZ = std::abs(point.z - min_.z);
      const float distMaxZ = std::abs(max_.z - point.z);

      float bestDist = distMinX;
      outClosest = { min_.x, point.y, point.z };
      outNormal = { -1.0f, 0.0f, 0.0f };

      if (distMaxX < bestDist)
      {
         bestDist = distMaxX;
         outClosest = { max_.x, point.y, point.z };
         outNormal = { 1.0f, 0.0f, 0.0f };
      }

      if (distMinY < bestDist)
      {
         bestDist = distMinY;
         outClosest = { point.x, min_.y, point.z };
         outNormal = { 0.0f, -1.0f, 0.0f };
      }

      if (distMaxY < bestDist)
      {
         bestDist = distMaxY;
         outClosest = { point.x, max_.y, point.z };
         outNormal = { 0.0f, 1.0f, 0.0f };
      }

      if (distMinZ < bestDist)
      {
         bestDist = distMinZ;
         outClosest = { point.x, point.y, min_.z };
         outNormal = { 0.0f, 0.0f, -1.0f };
      }

      if (distMaxZ < bestDist)
      {
         bestDist = distMaxZ;
         outClosest = { point.x, point.y, max_.z };
         outNormal = { 0.0f, 0.0f, 1.0f };
      }

      return true;
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

   bool AABBCollider::SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius,
      RaycastHit& outHit) const
   {
      const Math::Vector3 delta = to - from;
      const float dist = delta.Length();

      if (dist <= 0.001f)
         return false;

      const Math::Vector3 dir = delta.Normalized();

      Math::Vector3 min = GetWorldAABB().GetMin();
      Math::Vector3 max = GetWorldAABB().GetMax();

      min = min - Math::Vector3{ radius, radius, radius };
      max = max + Math::Vector3{ radius, radius, radius };

      Ray ray;
      ray.origin = from;
      ray.direction = dir;

      return RaycastExpandedAABB(ray, min, max, dist, outHit);
   }

   bool AABBCollider::RaycastExpandedAABB(const Ray& ray, const Math::Vector3& min, const Math::Vector3& max,
      float dist, RaycastHit& outHit) const
   {
      constexpr float EPSILON = 1e-6f;

      float tMin = 0.0f;
      float tMax = dist;

      Math::Vector3 hitNormal{ 0.0f, 0.0f, 0.0f };

      auto TestAxis = [&](float origin, float dir, float minVal, float maxVal, const Math::Vector3& axisNormal) -> bool
      {
         if (std::abs(dir) < EPSILON)
         {
            return origin >= minVal && origin <= maxVal;
         }

         float invD = 1.0f / dir;
         float t1 = (minVal - origin) * invD;
         float t2 = (maxVal - origin) * invD;

         Math::Vector3 normal = axisNormal;

         if (t1 > t2)
         {
            std::swap(t1, t2);
            normal = normal * -1.0f;
         }

         if (t1 > tMin)
         {
            tMin = t1;
            hitNormal = normal;
         }

         tMax = std::min(tMax, t2);

         return tMin <= tMax;
      };

      if (!TestAxis(ray.origin.x, ray.direction.x, min.x, max.x, Math::Vector3{ -1.0f, 0.0f, 0.0f }))
         return false;

      if (!TestAxis(ray.origin.y, ray.direction.y, min.y, max.y, Math::Vector3{ 0.0f, -1.0f, 0.0f }))
         return false;

      if (!TestAxis(ray.origin.z, ray.direction.z, min.z, max.z, Math::Vector3{ 0.0f, 0.0f, -1.0f }))
         return false;

      outHit.hit = true;
      outHit.t = tMin;
      outHit.point = ray.origin + ray.direction * tMin;
      outHit.normal = hitNormal;

      return true;
   }
}

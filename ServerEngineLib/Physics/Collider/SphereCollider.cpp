#include "pch.h"
#include "SphereCollider.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"

/*------------------
   SphereCollider
------------------*/

namespace SE::Physics
{
   SphereCollider::SphereCollider(const Vector3& center, float radius)
   {
      Set(center, radius);
   }

   ColliderType SphereCollider::GetType() const
   {
      return ColliderType::Sphere;
   }

   Collider* SphereCollider::Clone() const
   {
      return new SphereCollider(*this);
   }

   void SphereCollider::UpdateWorld(const Math::Vector3& position, float yaw)
   {
      worldCenter_ = position + RotateYaw(localCenter_, yaw); 
      worldRadius_ = localRadius_;
      
      RecalcWorldAABB();
   }

   void SphereCollider::Set(const Vector3& center, float radius)
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(radius >= 0.0f && "Sphere Radius must be >= 0");
      }
      
      localCenter_ = center;
      localRadius_ = SE::Math::Abs(radius);
      
      UpdateWorld(Vector3{0.0f, 0.0f, 0.0f}, 0.0f);
   }

   const AABBCollider& SphereCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   bool SphereCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(SE::Math::NearlyZero(ray.direction.LengthSq() - 1.0f, 1e-3f) && "Ray direction must be normalized");
      }
      
      const Vector3 m = ray.origin - worldCenter_;
      
      const float halfB = m.Dot(ray.direction);
      const float c = m.Dot(m) - (worldRadius_ * worldRadius_);
      
      const float disc = halfB * halfB - c;
      if (disc < 0.0f)  // 교차 없음
         return false;
      
      const float sqrtDisc = std::sqrt(disc);
      
      float t = -halfB - sqrtDisc;
      if (t < ray.tMin or t > ray.tMax) {
         t = -halfB + sqrtDisc;
         if (t < ray.tMin or t > ray.tMax) {
            return false;  // 교차 없음 (유효하지 않은 교차점)
         }
      }
      
      out.hit = true;
      out.t = t;
      out.point = ray.At(t);
      
      out.normal = (out.point - worldCenter_).Normalized(Vector3(0.0f, 1.0f, 0.0f));     // 노멀 단위벡터
                                                                                          // 만약 중심과 일치하면 Y축 방향으로 설정(임의 설정)
      out.collider = this;
      
      return true;
   }

   bool SphereCollider::SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius,
      RaycastHit& outHit) const
   {
      const Math::Vector3 delta = to - from;
      const float dist = delta.Length();

      if (dist <= 0.001f)
         return false;

      const Math::Vector3 dir = delta.Normalized();

      const Math::Vector3 center = GetCenter();
      const float expandedRadius = GetRadius() + radius;

      Ray ray;
      ray.origin = from;
      ray.direction = dir;

      if (!RaycastSphere(ray, center, expandedRadius, dist, outHit))
         return false;

      return true;
   }

   bool SphereCollider::Contains(const Vector3& point) const
   {
      const float dist = (point - worldCenter_).LengthSq();
      return dist <= RadiusSq();
   }

   bool SphereCollider::RaycastSphere(const Ray& ray, const Math::Vector3& center, float expandedRadius, float dist,
      RaycastHit& outHit) const
   {
      Math::Vector3 oc = ray.origin - center;

      float a = ray.direction.Dot(ray.direction);      // 보통 1 (normalize 되어 있으면)
      float b = 2.0f * oc.Dot(ray.direction);
      float c = oc.Dot(oc) - expandedRadius * expandedRadius;

      float discriminant = b * b - 4.0f * a * c;

      if (discriminant < 0.0f)
         return false;

      float sqrtD = std::sqrt(discriminant);

      float t1 = (-b - sqrtD) / (2.0f * a);
      float t2 = (-b + sqrtD) / (2.0f * a);

      float t = -1.0f;

      // 가장 가까운 양수 t 선택
      if (t1 >= 0.0f)
         t = t1;
      else if (t2 >= 0.0f)
         t = t2;

      if (t < 0.0f)
         return false;

      if (t > dist)
         return false;

      outHit.hit = true;
      outHit.t = t;
      outHit.point = ray.origin + ray.direction * t;

      Math::Vector3 normal = outHit.point - center;

      outHit.normal = normal.Normalized();

      return true;
   }

   void SphereCollider::RecalcWorldAABB()
   {
      const Vector3 r{worldRadius_, worldRadius_, worldRadius_};
      
      const Vector3 mn = worldCenter_ - r;
      const Vector3 mx = worldCenter_ + r;
      
      worldAABB_.SetMinMax(mn, mx);
   }
}

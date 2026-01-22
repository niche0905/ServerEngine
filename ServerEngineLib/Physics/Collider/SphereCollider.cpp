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

   bool SphereCollider::Intersect(const Collider& other, CollisionResult& out) const
   {
      // TODO: 충돌체 충돌 판정 방식 우선 개발
      
      return false;
   }

   void SphereCollider::Set(const Vector3& center, float radius)
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(SE::Math::NearlyZero(radius) && "Sphere Radius Zero");
      }
      
      center_ = center;
      radius_ = SE::Math::Abs(radius);
      
      RecalcWorldAABB();
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
      
      const Vector3 m = ray.origin - center_;
      
      const float halfB = m.Dot(ray.direction);
      const float c = m.Dot(m) - (radius_ * radius_);
      
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
      
      out.normal = (out.point - center_).Normalized(Vector3(0.0f, 1.0f, 0.0f));     // 노멀 단위벡터
                                                                                          // 만약 중심과 일치하면 Y축 방향으로 설정(임의 설정)
      out.collider = this;
      
      return true;
   }

   bool SphereCollider::Contains(const Vector3& point) const
   {
      const float dist = (point - center_).LengthSq();
      return dist <= RadiusSq();
   }

   void SphereCollider::RecalcWorldAABB()
   {
      const Vector3 r{radius_, radius_, radius_};
      
      const Vector3 mn = center_ - r;
      const Vector3 mx = center_ + r;
      
      worldAABB_.SetMinMax(mn, mx);
   }
}

#include "pch.h"
#include "SphereCollider.h"

/*------------------
   SphereCollider
------------------*/

namespace SE::Physics
{
   static inline float AbsF(float v) { return (v < 0.0f) ? -v : v; }
   
   //-----------------------------------------------------------------------------
   //-----------------------------------------------------------------------------
   
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
      center_ = center;
      radius_ = AbsF(radius);
      
      RecaleWorldAABB();
   }

   const AABBCollider& SphereCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   bool SphereCollider::Contains(const Vector3& point) const
   {
      const float dist = (center_ - point).LengthSq();
      return dist <= RadiusSq();
   }

   void SphereCollider::RecaleWorldAABB()
   {
      const Vector3 r{radius_, radius_, radius_};
      
      const Vector3 mn = center_ - r;
      const Vector3 mx = center_ + r;
      
      worldAABB_.SetMinMax(mn, mx);
   }
}

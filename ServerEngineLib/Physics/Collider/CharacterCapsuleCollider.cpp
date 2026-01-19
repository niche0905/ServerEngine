#include "pch.h"
#include "CharacterCapsuleCollider.h"

/*----------------------------
   CharacterCapsuleCollider
----------------------------*/

namespace SE::Physics
{
   CharacterCapsuleCollider::CharacterCapsuleCollider(const Vector3& base, float height, float radius)
   {
      Set(base, height, radius);
   }

   ColliderType CharacterCapsuleCollider::GetType() const
   {
      return ColliderType::CharacterCapsule;
   }

   CharacterCapsuleCollider* CharacterCapsuleCollider::Clone() const
   {
      return new CharacterCapsuleCollider(*this);
   }

   bool CharacterCapsuleCollider::Intersect(const Collider& other, CollisionResult& out) const
   {
      // TODO: 나중에 채우길
      
      return false;
   }

   void CharacterCapsuleCollider::Set(const Vector3& base, float height, float radius)
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(radius >= 0.0f && "Character Capsule radius must be >= 0");
         assert(height >= 2.0f * radius && "Character Capsule height must be >= 2*radius");
      }
      
      base_ = base;
      height_ = height;
      radius_ = SE::Math::Abs(radius);
      
      if (height_ < 2.0f * radius_) height_ = 2.0f * radius_;  // 캡슐 크기 유효하게
      
      RecalcWorldAABB();
   }

   const AABBCollider& CharacterCapsuleCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   Vector3 CharacterCapsuleCollider::ClosestPointOnSegment(const Vector3& point) const
   {
      const float yA = base_.y + radius_;
      const float yB = base_.y + (height_ - radius_);
      
      float y = point.y;
      if (y < yA) y = yA;
      if (y > yB) y = yB;
      
      return Vector3{ base_.x, y, base_.z };
   }

   Vector3 CharacterCapsuleCollider::ClosestPoint(const Vector3& point) const
   {
      const Vector3 c = ClosestPointOnSegment(point);
      
      Vector3 v = point - c;
      const float distSq = v.LengthSq();
      
      // point가 중심선(Seqment) 위의 점이라면 가까운 표면을 정할 수 없다
      if (distSq <= 1e-12f)
         return c;
      
      const float rSq = radius_ * radius_;
      if (distSq <= rSq)
         return point;  // 캡슐 내부라면 그대로 반환
      
      const Vector3 n = v.Normalized();
      return c + (n * radius_);
   }

   float CharacterCapsuleCollider::DistanceSqToSegment(const Vector3& point) const
   {
      const Vector3 c = ClosestPointOnSegment(point);
      return (point - c).LengthSq();
   }

   void CharacterCapsuleCollider::RecalcWorldAABB()
   {
      const Vector3 mn{
         base_.x - radius_,
         base_.y,
         base_.z - radius_
     };

      const Vector3 mx{
         base_.x + radius_,
         base_.y + height_,
         base_.z + radius_
     };

      worldAABB_.SetMinMax(mn, mx);
   }
}

#pragma once
#include "AABBCollider.h"
#include "Collider.h"

/*----------------------------
   CharacterCapsuleCollider
----------------------------*/
//
// CharacterCapsuleCollider는 캡슐 형태의 충돌체입니다
// Up vector가 고정되어 캐릭터의 이동, 충돌 등의 검사를 할 때 사용합니다
//

namespace SE::Physics
{
   class CharacterCapsuleCollider : public Collider
   {
   public:
      using Vector3 = SE::Math::Vector3;
      
   public:
      CharacterCapsuleCollider() = default;
      virtual ~CharacterCapsuleCollider() = default;
      
      CharacterCapsuleCollider(const Vector3& base, float height, float radius);
      
      virtual ColliderType GetType() const override;
      virtual Collider* Clone() const override;
      
      virtual bool Intersect(const Collider& other, CollisionResult& out) const override;
      
   public:
      void Set(const Vector3& base, float height, float radius);
      
      const Vector3& GetBase() const { return base_; }
      float GetHeight() const { return height_; }
      float GetRadius() const { return radius_; }
      
      float GetCylinderHeight() const { return (height_ - 2.0f * radius_); }
      Vector3 GetCenter() const { return (base_ + Vector3{0.0f, height_ * 0.5f, 0.0f}); }
      
      Vector3 GetPointA() const { return (base_ + Vector3{0.0f, radius_, 0.0f}); }
      Vector3 GetPointB() const { return (base_ + Vector3{0.0f, height_ - radius_, 0.0f}); }
      
      float GetBottomY() const { return base_.y; }
      float GetTopY() const { return base_.y + height_; }
      
      const AABBCollider& GetWorldAABB() const override;
      
      virtual bool Raycast(const Ray& ray, RaycastHit& out) const override;
      
      Vector3 ClosestPointOnSegment(const Vector3& point) const;
      Vector3 ClosestPoint(const Vector3& point) const;
      float DistanceSqToSegment(const Vector3& point) const;

   private:
      void RecalcWorldAABB();
      
   private:
      Vector3 base_{}; // pivot
      float height_{};
      float radius_{};
      
      AABBCollider worldAABB_{};
    
   };
   
}
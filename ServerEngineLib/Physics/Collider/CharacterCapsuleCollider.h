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
      
      virtual void UpdateWorld(const Math::Vector3& position, float yaw) override;
      
   public:
      void Set(const Vector3& base, float height, float radius);
      
      const Vector3& GetBase() const { return worldBase_; }
      float GetHeight() const { return worldHeight_; }
      float GetRadius() const { return worldRadius_; }
      
      float GetCylinderHeight() const { return (worldHeight_ - 2.0f * worldRadius_); }
      Vector3 GetCenter() const { return (worldBase_ + Vector3{0.0f, 0.0f, worldHeight_ * 0.5f}); }
      
      Vector3 GetPointA() const { return (worldBase_ + Vector3{0.0f, 0.0f, worldRadius_}); }
      Vector3 GetPointB() const { return (worldBase_ + Vector3{0.0f, 0.0f, worldHeight_ - worldRadius_}); }
      
      float GetBottomY() const { return worldBase_.z; }
      float GetTopY() const { return worldBase_.z + worldHeight_; }
      
      const AABBCollider& GetWorldAABB() const override;
      
      virtual bool Raycast(const Ray& ray, RaycastHit& out) const override;
      
      Vector3 ClosestPointOnSegment(const Vector3& point) const;
      Vector3 ClosestPoint(const Vector3& point) const;
      float DistanceSqToSegment(const Vector3& point) const;

   private:
      void RecalcWorldAABB();
      
   private:
      Vector3 localBase_{}; // pivot
      float localHeight_{};
      float localRadius_{};
      
      Vector3 worldBase_{};
      float worldHeight_{};
      float worldRadius_{};
      
      AABBCollider worldAABB_{};
    
   };
   
}
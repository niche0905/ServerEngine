#pragma once
#include "AABBCollider.h"
#include "Collider.h"

/*---------------
   OBBCollider
---------------*/
//
// OBBCollider는 OBB 방식을 채택한 충돌체 입니다
//

namespace SE::Physics
{
   class OBBCollider : public Collider
   {
   public:
      using Vector3 = SE::Math::Vector3;
      
   public:
      OBBCollider() = default;
      virtual ~OBBCollider() = default;
      
      OBBCollider(const Vector3& center,
         const Vector3& halfExtent,
         const Vector3& axisX,
         const Vector3& axisY,
         const Vector3& axisZ);
      
      virtual ColliderType GetType() const override;
      virtual Collider* Clone() const override;
      
      virtual void UpdateWorld(const Math::Vector3& position, float yaw) override;
      
   public:
      void Set(const Vector3& center,
         const Vector3& halfExtent,
         const Vector3& axisX,
         const Vector3& axisY,
         const Vector3& axisZ);
      
      const Vector3& GetCenter() const { return  worldCenter_; }
      const Vector3& GetHalfExtent() const { return  worldHalf_; }
      const Vector3& GetAxisX() const { return  worldAxis_[0]; }
      const Vector3& GetAxisY() const { return  worldAxis_[1]; }
      const Vector3& GetAxisZ() const { return  worldAxis_[2]; }
      
      virtual bool ContainsPoint(const Math::Vector3& point) const override;
      virtual bool ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest, Math::Vector3& outNormal) const override;
      
      const AABBCollider& GetWorldAABB() const override;
      
      virtual bool Raycast(const Ray& ray, RaycastHit& out) const override;
      virtual bool SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius, RaycastHit& outHit) const override;
      
      Vector3 ClosestPoint(const Vector3& point) const;
      
   private:
      void RecalcWorldAABB();
      
   private:
      Vector3 localCenter_{};
      Vector3 localHalf_{};
      Vector3 localAxis_[3]{};
      
      Vector3 worldCenter_{};
      Vector3 worldHalf_{};
      Vector3 worldAxis_[3]{};
      
      AABBCollider worldAABB_{};
    
   };
   
}



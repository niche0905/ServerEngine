#pragma once
#include "AABBCollider.h"
#include "Collider.h"
#include "Math/Vector.h"

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
      virtual Collider* Clone() const;
      
      virtual bool Intersect(const Collider& other, CollisionResult& out) const override;
      
   public:
      void Set(const Vector3& center,
         const Vector3& halfExtent,
         const Vector3& axisX,
         const Vector3& axisY,
         const Vector3& axisZ);
      
      const Vector3& GetCenter() const { return  center_; }
      const Vector3& GetHalfExtent() const { return  half_; }
      const Vector3& GetAxisX() const { return  axis_[0]; }
      const Vector3& GetAxisY() const { return  axis_[1]; }
      const Vector3& GetAxisZ() const { return  axis_[2]; }
      
      const AABBCollider& GetWorldAABB() const;
      
      Vector3 ClosestPoint(const Vector3& point) const;
      
   private:
      void RecalcWorldAABB();
      
   private:
      Vector3 center_{};
      Vector3 half_{};
      Vector3 axis_[3]{};
      
      AABBCollider worldAABB_;
    
   };
   
}



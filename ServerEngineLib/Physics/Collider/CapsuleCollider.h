#pragma once
#include "Collider.h"
#include "AABBCollider.h"

/*-------------------
   CapsuleCollider
-------------------*/
//
// CapsuleCollider는 캡슐 형태의 충돌체입니다
//

namespace SE::Physics
{
    class CapsuleCollider : public Collider
    {
    public:
        using Vector3 = SE::Math::Vector3;
        
    public:
        CapsuleCollider() = default;
        virtual ~CapsuleCollider() = default;
        
        CapsuleCollider(const Vector3& pointA, const Vector3& pointB, float radius);
        
        virtual ColliderType GetType() const override;
        virtual Collider* Clone() const override;
        
        virtual void UpdateWorld(const Math::Vector3& position, float yaw) override;
        
    public:
        void Set(const Vector3& pointA, const Vector3& pointB, float radius);
        
        const Vector3& GetPointA() const { return worldPointA_; }
        const Vector3& GetPointB() const { return worldPointB_; }
        float GetRadius() const { return worldRadius_; }
        
        float RadiusSq() const { return worldRadius_ * worldRadius_; }
        
        Vector3 GetCenter() const { return (worldPointA_ + worldPointB_) * 0.5f; }
        Vector3 GetAxis() const { return dir_; }
        float GetHalfLen() const { return halfLen_; }
        float GetSegmentLen() const { return halfLen_ * 2; }
        
        virtual bool ContainsPoint(const Math::Vector3& point) const override;
        virtual bool ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest, Math::Vector3& outNormal) const override;
        
        const AABBCollider& GetWorldAABB() const override;
        
        virtual bool Raycast(const Ray& ray, RaycastHit& out) const override;
        virtual bool SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius, RaycastHit& outHit) const override;
        
        Vector3 ClosestPointOnSegment(const Vector3& point) const;
        Vector3 ClosestPoint(const Vector3& point) const;
        float DistanceSqToSegment(const Vector3& point) const;

    private:
        void RecalcWorldAABB();
        void RecalcDerived();
        
    private:
        Vector3 localPointA_{};
        Vector3 localPointB_{};
        float localRadius_{};
        
        Vector3 worldPointA_{};
        Vector3 worldPointB_{};
        float worldRadius_{};
        
        Vector3 dir_{};
        float halfLen_{};
        
        AABBCollider worldAABB_{};
    
    };
    
}
#pragma once
#include "AABBCollider.h"
#include "Collider.h"

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
        
        virtual bool Intersect(const Collider& other, CollisionResult& out) const override;
        
    public:
        void Set(const Vector3& pointA, const Vector3& pointB, float radius);
        
        const Vector3& GetPointA() const { return pointA_; }
        const Vector3& GetPointB() const { return pointB_; }
        float GetRadius() const { return radius_; }
        
        float RadiusSq() const { return radius_ * radius_; }
        
        Vector3 GetCenter() const { return (pointA_ + pointB_) * 0.5f; }
        Vector3 GetAxis() const { return dir_; }
        float GetHalfLen() const { return halfLen_; }
        float GetSegmentLen() const { return halfLen_ * 2; }
        
        const AABBCollider& GetWorldAABB() const;
        
        Vector3 ClosestPointOnSegment(const Vector3& point) const;
        Vector3 ClosestPoint(const Vector3& point) const;
        float DistanceSqToSegment(const Vector3& point) const;
        
    private:
        void RecalcWorldAABB();
        void RecalcDerived();
        
    private:
        Vector3 pointA_{};
        Vector3 pointB_{};
        float radius_{};
        
        Vector3 dir_{};
        float halfLen_{};
        
        AABBCollider worldAABB_;
    
    };
    
}
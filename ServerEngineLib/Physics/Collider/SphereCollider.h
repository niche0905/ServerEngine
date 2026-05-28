#pragma once
#include "AABBCollider.h"
#include "Collider.h"

/*------------------
   SphereCollider
------------------*/
//
// SphereCollider는 구체 형태의 충돌체입니다
//

namespace SE::Physics
{
    class SphereCollider : public Collider
    {
    public:
        using Vector3 = SE::Math::Vector3;
        
    public:
        SphereCollider() = default;
        virtual ~SphereCollider() = default;
        
        SphereCollider(const Vector3& center, float radius);
        
        virtual ColliderType GetType() const override;
        virtual Collider* Clone() const override;
        
        virtual void UpdateWorld(const Math::Vector3& position, float yaw) override;
        
    public:
        void Set(const Vector3& center, float radius);
        
        const Vector3& GetCenter() const { return localCenter_; }
        float GetRadius() const { return localRadius_; }
        
        virtual bool ContainsPoint(const Math::Vector3& point) const override;
        virtual bool ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest, Math::Vector3& outNormal) const override;
        
        const AABBCollider& GetWorldAABB() const override;
        
        virtual bool Raycast(const Ray& ray, RaycastHit& out) const override;
        virtual bool SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius, RaycastHit& outHit) const override;
        
        bool Contains(const Vector3& point) const;
        float RadiusSq() const { return localRadius_ * localRadius_; }
        bool RaycastSphere(const Ray& ray, const Math::Vector3& center, float expandedRadius, float dist, RaycastHit& outHit) const;
        
    private:
        void RecalcWorldAABB();
        
    private:
        Vector3 localCenter_{};
        float localRadius_{};
        
        Vector3 worldCenter_{};
        float worldRadius_{};
        
        AABBCollider worldAABB_{};
    
    };
    
}


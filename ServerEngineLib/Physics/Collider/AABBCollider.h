#pragma once
#include "Collider.h"

/*----------------
   AABBCollider
----------------*/
//
// AABBCollider는 AABB 방식을 채택한 충돌체 입니다
//

namespace SE::Physics
{
    class AABBCollider : public Collider
    {
    public:
        using Vector3 = SE::Math::Vector3;
        
    public:
        AABBCollider() = default;
        virtual ~AABBCollider() = default;
        
        AABBCollider(const Vector3& minPoint, const Vector3& maxPoint);
    
        virtual ColliderType GetType() const override;
        virtual Collider* Clone() const override;
        
        virtual void UpdateWorld(const Math::Vector3& position, float yaw) override;
        
    public:
        void SetMinMax(const Vector3& minPoint, const Vector3& maxPoint);
        
        const Vector3& GetMin() const { return min_; }
        const Vector3& GetMax() const { return max_; }
        const Vector3& GetCenter() const { return worldCenter_; }
        const Vector3& GetExtent() const { return worldExtent_; }
        
        bool Contains(const Vector3& point) const;
        bool Overlaps(const AABBCollider& other) const;
        // void Expand(float margin);
        static AABBCollider Union(const AABBCollider& a, const AABBCollider& b);
        
        virtual bool ContainsPoint(const Math::Vector3& point) const override;
        virtual bool ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest, Math::Vector3& outNormal) const override;
        
        const AABBCollider& GetWorldAABB() const override;
        
        virtual bool Raycast(const Ray& ray, RaycastHit& out) const override;
        virtual bool SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius, RaycastHit& outHit) const override;
        
        bool RaycastExpandedAABB(const Ray& ray, const Math::Vector3& min, const Math::Vector3& max, float dist, RaycastHit& outHit) const;
        
    private:
        Vector3 localCenter_{};
        Vector3 localExtent_{};
        
        Vector3 min_{};
        Vector3 max_{};
        
        // cache
        Vector3 worldCenter_{};
        Vector3 worldExtent_{};
    
    };

}


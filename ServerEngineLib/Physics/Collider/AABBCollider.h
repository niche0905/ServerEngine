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
        
        virtual bool Intersect(const Collider& other, CollisionResult& out) const override;
        
    public:
        void SetMinMax(const Vector3& minPoint, const Vector3& maxPoint);
        
        const Vector3& GetMin() const { return min_; }
        const Vector3& GetMax() const { return max_; }
        const Vector3& GetCenter() const { return center_; }
        const Vector3& GetExtent() const { return extent_; }
        
        bool Contains(const Vector3& point) const;
        bool Overlaps(const AABBCollider& other) const;
        void Expand(float margin);
        static AABBCollider Union(const AABBCollider& a, const AABBCollider& b);
        
    private:
        void RecalcCache();
        
    private:
        Vector3 min_{};
        Vector3 max_{};
        
        // cache
        Vector3 center_{};
        Vector3 extent_{};
    
    };

}


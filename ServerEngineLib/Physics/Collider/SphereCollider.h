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
        
    public:
        void Set(const Vector3& center, float radius);
        
        const Vector3& GetCenter() const { return center_; }
        float GetRadius() const { return radius_; }
        
        const AABBCollider& GetWorldAABB() const override;
        
        virtual bool Raycast(const Ray& ray, RaycastHit& out) const override;
        
        bool Contains(const Vector3& point) const;
        float RadiusSq() const { return radius_ * radius_; }
        
    private:
        void RecalcWorldAABB();
        
    private:
        Vector3 center_{};
        float radius_{};
        
        AABBCollider worldAABB_{};
    
    };
    
}


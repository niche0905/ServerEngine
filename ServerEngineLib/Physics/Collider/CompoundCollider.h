#pragma once
#include "Collider.h"
#include "AABBCollider.h"

/*--------------------
   CompoundCollider
--------------------*/
//
// CompoundCollider는 복합적인 프리미티브를 가져야 할 필요성이 있는 충돌체입니다
//

namespace SE::Physics
{
    class CompoundCollider : public Collider
    {
    public:
        using Vector3 = SE::Math::Vector3;
        using ColliderRef = std::unique_ptr<Collider>;
        
    public:
        CompoundCollider() = default;
        virtual ~CompoundCollider() = default;
        
        virtual ColliderType GetType() const override;
        virtual Collider* Clone() const override;
        
        virtual void UpdateWorld(const Math::Vector3& position, float yaw) override;
        
        virtual bool Intersect(const Collider& other, CollisionResult& out) const override;
        
    public:
        void Reserve(size_t n);
        void Clear();
        
        void Add(ColliderRef collider);
        
        template <typename T, typename... Args>
        T& Emplace(Args&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *ptr;
            colliders_.push_back(std::move(ptr));
            dirtyAABB_ = true;
            return ref;
        }
        
        virtual bool ContainsPoint(const Math::Vector3& point) const override;
        virtual bool ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest, Math::Vector3& outNormal) const override;
        
        const AABBCollider& GetWorldAABB() const override;
        
        virtual bool Raycast(const Ray& ray, RaycastHit& out) const override;
        virtual bool SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius, RaycastHit& outHit) const override;
        
    private:
        void RecalcWorldAABB() const;
        
    private:
        std::vector<ColliderRef> colliders_;
        
        mutable bool dirtyAABB_{true};
        mutable AABBCollider worldAABB_{};
    
    };
   
}


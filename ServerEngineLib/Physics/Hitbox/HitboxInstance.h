#pragma once
#include <vector>
#include "HitboxPart.h"
#include "HitResult.h"
#include "Physics/Collider/AABBCollider.h"

/*------------------
   HitboxInstance
------------------*/
//
// HitboxInstance는 한 객체가 가진 Hitbox collider들을 런타임 월드 상태로 갱신합니다.
//

namespace SE::Physics
{
    class Collider;
    struct Ray;
}

namespace SE::Physics::Hit
{
    class HitboxInstance
    {
    public:
        using Vector3 = SE::Math::Vector3;
        
    public:
        void Bind(std::vector<HitboxPart> parts);
        void Clear();
        bool IsBound() const { return !parts_.empty(); }
        void Update(const Vector3& rootPos, float yawDegrees);
        
        const AABBCollider& GetWorldAABB() const;
        const std::vector<HitboxPart>& GetParts() const { return parts_; }
        
        bool Raycast(const Ray& ray, HitResult& out) const;
        bool SphereCast(const Vector3& from, const Vector3& to, float radius, HitResult& out) const;
        bool Intersect(const Collider& other, HitResult* out = nullptr) const;

    private:
        std::vector<HitboxPart> parts_;
        AABBCollider worldAABB_;
    };
}

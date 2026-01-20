#pragma once
#include "CollsionResult.h"

/*------------
   Collider
------------*/
//
// Collider는 서버 컨텐츠에서 사용하기 위해 만든 충돌체 클래스 입니다
//


namespace SE::Physics
{
    struct CollisionResult;
    class AABBCollider;
    
    enum class ColliderType : uint8
    {
        AABB,
        OBB,
        Sphere,
        Capsule,
        CharacterCapsule,
        
        Compound,
    };
    
    class Collider
    {
    public:
        virtual ~Collider() = default;
        
        virtual ColliderType GetType() const = 0;
        virtual Collider* Clone() const = 0;
        
        virtual bool Intersect(const Collider& other, CollisionResult& out) const = 0;
        
        virtual const AABBCollider& GetWorldAABB() const = 0;
        
        // TODO: Ray 부터 만들고 아래 구성
        // virtual bool Raycast(const class Ray& ray, class RayHit& out) const;
    };
    
}
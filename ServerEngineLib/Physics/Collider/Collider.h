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
    
    enum class ColliderType : uint8
    {
        AABB,
        OBB,
        Sphere,
        Capsule,
    };
    
    class Collider
    {
    public:
        virtual ~Collider() = default;
        
        virtual ColliderType GetType() const = 0;
        virtual bool Intersect(const Collider& other, CollisionResult& out) const = 0;
    };
    
}
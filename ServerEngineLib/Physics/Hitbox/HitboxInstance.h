#pragma once
#include "HitboxPart.h"
#include "Physics/Collider/AABBCollider.h"

/*------------------
   HitboxInstance
------------------*/
//
// HitboxInstance는 한 객체(Hitbox를 가져야 할) Hitbox 정보를 가지게 될 클래스입니다
//

namespace SE::Physics::Hit
{
    class HitboxAsset
    {
    public:
        std::vector<HitboxPart> parts;
        
    };
    
    class HitboxInstance
    {
    public:
        using Vector3 = SE::Math::Vector3;
        
    public:
        void Bind(const HitboxAsset* asset);
        void Update(const Vector3& rootPos, float yawRadians);
        
        const AABBCollider& GetWorldAABB() const;
        
        // TODO: Ray 먼저 작성하기 SE::Physics의 영역이다
        bool Raycast(Ray& ray, RaycastHit& out) const;

    private:
        const HitboxAsset* asset_ = nullptr;
        
        // TODO: 런타임 캐시가 필요하게 된다면 사용할 것
        struct WorldPart
        {
            HitShapeType type;
            HitGroup group;
            float mult;
            
        };
        
        std::vector<WorldPart> worldParts_;
        AABBCollider worldAABB_;
    
    };
    
}

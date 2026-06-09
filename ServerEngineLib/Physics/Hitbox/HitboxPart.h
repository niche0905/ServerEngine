#pragma once
#include <memory>
#include "Utils/Types.h"
#include "HitGroup.h"
#include "Physics/Collider/Collider.h"

/*--------------
   HitboxPart
--------------*/
//
// HitboxPart는 피격 가능한 한 부위와 해당 collider prototype을 가집니다.
//

namespace SE::Physics::Hit
{
    enum class HitShapeType : uint8
    {
        Sphere,
        Capsule,
        OBB,
    };
    
    struct HitboxPart
    {
        HitboxPart() = default;
        ~HitboxPart() = default;

        HitboxPart(const HitboxPart&) = delete;
        HitboxPart& operator=(const HitboxPart&) = delete;
        
        HitboxPart(HitboxPart&&) noexcept = default;
        HitboxPart& operator=(HitboxPart&&) noexcept = default;

        HitShapeType type{ HitShapeType::Sphere };
        HitGroup group{ HitGroup::Unknown };
        float damageMultiplier{1.0f};
        std::unique_ptr<Collider> collider;
    };
}

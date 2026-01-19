#pragma once
#include "HitGroup.h"

/*--------------
   HitboxPart
--------------*/
//
// HitboxPart는 피격 가능한 한 부위의 정보 집합입니다
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
        HitShapeType type;
        HitGroup group;
        float damageMultiplier;
        
        SE::Math::Vector3 localOffset;  // root에서의 중심/기준점 으로부터
        float radius;
    };
    
}
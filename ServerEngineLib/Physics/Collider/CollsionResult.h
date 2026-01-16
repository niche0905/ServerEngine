#pragma once
#include "Math/Vector.h"

/*-------------------
   CollisionResult
-------------------*/
//
// CollisionResult는 충돌 검사 후 반환해야 할 정보를 담을 구조체입니다
//

namespace SE::Physics
{
    using Math::Vector3;
    
    struct CollisionResult
    {
        bool hit = false;
        Vector3 point;
        Vector3 normal;
        float penetration;
    };
    
}

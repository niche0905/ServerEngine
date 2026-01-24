#pragma once
#include "Physics/Collider/Collider.h"

/*---------------
   SwapWrapper
---------------*/
//
// SwapWrapper는 충돌 판정 함수에서 콜라이더의 순서를 바꿔 호출하는 유틸리티 함수입니다
// 구현의 편의를 위해 사용됩니다
//

namespace SE::Physics::Narrowphase
{
    using IntersectFn = bool(*)(const Collider&, const Collider&, CollisionResult&);
    
    bool SwapWrapper(const Collider& a, const Collider& b, CollisionResult& out, IntersectFn fn);
    
}

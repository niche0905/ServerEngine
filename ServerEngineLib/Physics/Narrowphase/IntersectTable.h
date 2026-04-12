#pragma once
#include "Physics/Collider/Collider.h"

/*---------------
   Narrowphase
---------------*/
//
// Narrowphase 네임스페이스는 충돌 판정의 세부 구현을 담당합니다
// CollisionResult의 normal은 a를 b에서 밀어내는 방향으로 통일하였습니다
//

namespace SE::Physics::NarrowPhase
{
    // IntersectFn는 실제 충돌 판정을 수행하는 함수 포인터 타입입니다
    using IntersectFn = bool(*)(const Collider&, const Collider&, CollisionResult&);
    
    // LUT 초기화 (엔진 시작 시 1회 초기화)
    void InitIntersectTable();
    
    // LUT 접근자
    IntersectFn GetIntersectFn(ColliderType a, ColliderType b);
}

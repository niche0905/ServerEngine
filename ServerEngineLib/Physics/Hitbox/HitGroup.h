#pragma once

/*------------
   HitGroup
------------*/
//
// HitGroup은 슈팅에 피격될 수 있는 HitboxPart의 Type입니다
//

namespace SE::Physics::Hit
{
    enum class HitGroup : uint8
    {
        Head,   // 머리
        Torso,  // 몸통
        Arms,   // 팔
        Legs,   // 다리
        
        NotHurtBox,  // 히트박스는 맞았지만, 데미지를 입히지 않는 부분 (예: 무기, 장갑 등)
        
        Unknown // 알 수 없음...
    };
   
}

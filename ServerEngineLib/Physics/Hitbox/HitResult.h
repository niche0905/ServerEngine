#pragma once
#include "HitGroup.h"

/*-------------
   HitResult
-------------*/
//
// 전투(Hitbox) 전용 Raycast 결과 구조체입니다
//

namespace SE::Physics::Hit
{
    struct HitResult
    {
    public:
        using Vector3 = SE::Math::Vector3;
        
        bool hit{false};                    // 히트 여부
        
        float t{0.0f};                      // 레이캐스트 시 t값
        
        Vector3 point{};                    // 히트 위치
        Vector3 normal{};                   // 히트 법선
        
        HitGroup group{HitGroup::Unknown};  // 히트 그룹
        float damageMultiplier{1.0f};       // 데미지 배율
        
        uint16 partIndex{0};               // 히트한 부위 인덱스(없으면 0)
        
        void Reset()
        {
            hit = false;
            t = 0.0f;
            point = Vector3{};
            normal = Vector3{};
            group = HitGroup::Unknown;
            damageMultiplier = 1.0f;
            partIndex = 0;
        }
    };
}

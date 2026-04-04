#pragma once
#include "Math/Vector.h"

struct ZoneCircle
{
    SE::Math::Vector3 Center{};
    float Radius = 0.0f;
    
    bool Contains(SE::Math::Vector3 point) const
    {
        SE::Math::Vector3 temp = point - Center;
        temp.y = 0; // 수평 평면에서의 거리 계산을 위해 y 좌표를 무시
        return temp.LengthSq() <= Radius * Radius;
    }
};

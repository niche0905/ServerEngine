#pragma once
#include "Math/Vector.h"

struct ZoneCircle
{
    SE::Math::Vector3 center{};
    float radius = 0.0f;
    
    bool Contains(SE::Math::Vector3 point) const
    {
        SE::Math::Vector3 temp = point - center;
        temp.z = 0; // 수평 평면에서의 거리 계산을 위해 z 좌표를 무시
        return temp.LengthSq() <= radius * radius;
    }
};

struct ZoneBounds
{
    SE::Math::Vector3 center{};
    SE::Math::Vector3 extent{};
    
    SE::Math::Vector3 GetMin() const
    {
        return center - (extent * 0.5f);
    }
    
    SE::Math::Vector3 GetMax() const
    {
        return center + (extent * 0.5f);
    }
    
    float GetMaxInscribedCircleRadius() const
    {
        return std::min(extent.x, extent.y) * 0.5f;
    }
};

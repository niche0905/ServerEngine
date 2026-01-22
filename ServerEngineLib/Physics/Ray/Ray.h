#pragma once

/*-------
   Ray
-------*/
//
// Ray는 광선을 나타내는 구조체입니다
// Raycast로 충돌을 검사할 때 사용됩니다
//

namespace SE::Physics
{
    struct Ray
    {
    public:
        using Vector3 = SE::Math::Vector3;
        
    public:
        Vector3 origin{};    // 광선의 시작점
        Vector3 direction{}; // 광선의 방향(단위 벡터)
        
        float tMin = 0.0f;   // 시작 파라미터
        float tMax = 1e30f;  // 끝 파라미터 (사거리)
        
        Ray() = default;
        
        Ray(const Vector3& o, const Vector3& d, float maxDist)
            : origin(o), direction(d), tMin(0.0f), tMax(maxDist)
        {
        }
        
        Ray(const Vector3& o, const Vector3& d, float minDist, float maxDist)
            : origin(o), direction(d), tMin(minDist), tMax(maxDist)
        {
        }
        
        // t 위치의 실제 월드 좌표
        Vector3 At(float t) const
        {
            return origin + direction * t;
        }
    };
    
}
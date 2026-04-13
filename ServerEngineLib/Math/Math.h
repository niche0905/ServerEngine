#pragma once
#include <numbers>
#include "Vector.h"

/*--------
   Math
--------*/
//
// 공통 수학 함수
//

namespace SE::Math
{
    inline float Abs(float v) { return  (v < 0.0f) ? -v : v; }
    inline float Min(float a, float b) { return (a < b) ? a : b; }
    inline float Max(float a, float b) { return (a > b) ? a : b; }
    inline float Clamp(float v, float lo, float hi) { return Max(lo, Min(v, hi)); }
    
    inline float Sqr(float v) { return v * v; }
    
    inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    
    inline bool NearlyZero(float v, float eps = 1e-4f) { return Abs(v) <= eps; }
    inline bool NearlyEqual(float a, float b, float eps = 1e-4f) { return Abs(a - b) <= eps; }
    
    inline float DegreesToRadians(float degrees) { return degrees * (std::numbers::pi_v<float> / 180.0f); }
    inline SE::Math::Vector3 RotateYaw(const SE::Math::Vector3& v, float yawDegrees)
    {
        const float rad = DegreesToRadians(yawDegrees);
        const float c = std::cos(rad);
        const float s = std::sin(rad);
        
        return SE::Math::Vector3{
        v.x * c - v.y * s, 
        v.x * s + v.y * c,
        v.z
        };
    }
}

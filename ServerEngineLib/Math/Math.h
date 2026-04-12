#pragma once
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
    
    inline SE::Math::Vector3 RotateYaw(const SE::Math::Vector3& v, float yawDegrees)
    {
        const float c = std::cos(yawDegrees);
        const float s = std::sin(yawDegrees);
        
        return SE::Math::Vector3{
        v.x * c - v.z * s, 
        v.y,
        v.x * s + v.z * c
        };
    }
}

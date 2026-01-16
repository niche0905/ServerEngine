#pragma once

/*-----------
   Vector3
-----------*/
//
// Vector3는 Server Engine 내/외부에서 사용할 가능성이 있는 좌표 표현 Vector입니다
//

namespace SE::Math
{
    struct Vector3
    {
        float x, y, z;

        constexpr Vector3() : x(0), y(0), z(0) {}
        constexpr Vector3(float x, float y, float z)
            : x(x), y(y), z(z) {}

        Vector3 operator+(const Vector3& rhs) const
        {
            return { x + rhs.x, y + rhs.y, z + rhs.z };
        }

        Vector3 operator-(const Vector3& rhs) const
        {
            return { x - rhs.x, y - rhs.y, z - rhs.z };
        }

        Vector3 operator*(float s) const
        {
            return { x * s, y * s, z * s };
        }
        
        // QUES: Dot을 따로 뺄 순 없을까? Dot(a, b);
        float Dot(const Vector3& other) const
        {
            return x * other.x + y * other.y + z * other.z;
        }
        
        float LengthSq() const
        {
            return x * x + y * y + z * z;
        }
        
        float Length() const
        {
            return std::sqrt(LengthSq());
        }
        
        Vector3 Normalized(const Vector3& fallback = {0, 0, 0}) const
        {
            float lenSq = LengthSq();
            if (lenSq <= 1e-12f)
                return fallback;    // 값이 너무 작으면 무시 (0으로 생각), 인자가 있다면 해당 값으로 반환
            
            float invLen = 1.0f / lenSq;
            return { x * invLen, y * invLen, z * invLen };
        }
    };
}

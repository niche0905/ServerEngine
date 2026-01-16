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
    };
}

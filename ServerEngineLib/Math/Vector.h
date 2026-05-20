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
        
        static constexpr Vector3 Zero() { return { 0.0f, 0.0f, 0.0f }; }
        static constexpr Vector3 One()  { return { 1.0f, 1.0f, 1.0f }; }
        static constexpr Vector3 Up()   { return { 0.0f, 0.0f, 1.0f }; }

        constexpr Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
        constexpr Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

        Vector3 operator+(const Vector3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
        Vector3 operator-(const Vector3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
        Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }
        Vector3 operator/(float s) const 
        { 
            // 0 나누기 방지 (서버 안정성 확보)
            if (std::abs(s) < 1e-12f) return Zero();
            float inv = 1.0f / s;
            return { x * inv, y * inv, z * inv }; 
        }
        
        Vector3& operator+=(const Vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
        Vector3& operator-=(const Vector3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
        Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
        Vector3& operator/=(float s) 
        { 
            if (std::abs(s) < 1e-12f) { *this = Zero(); return *this; }
            float inv = 1.0f / s; x *= inv; y *= inv; z *= inv; return *this; 
        }
        
        Vector3 operator-() const { return { -x, -y, -z }; }
        
        bool operator==(const Vector3& rhs) const { return (x == rhs.x) && (y == rhs.y) && (z == rhs.z); }
        bool operator!=(const Vector3& rhs) const { return !(*this == rhs); }
        
        float Dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }
        Vector3 Cross(const Vector3& rhs) const
        {
            return {
                y * rhs.z - z * rhs.y,
                z * rhs.x - x * rhs.z,
                x * rhs.y - y * rhs.x
            };
        }
        
        float LengthSq() const { return x * x + y * y + z * z; }
        float Length2DSq() const { return x * x + y * y; }
        float Length() const { return std::sqrt(LengthSq()); }
        float Length2D() const { return std::sqrt(Length2DSq()); }
        
        Vector3 Normalized(const Vector3& fallback = Zero()) const
        {
            float lenSq = LengthSq();
            if (lenSq <= 1e-12f) return fallback;
            
            float invLen = 1.0f / std::sqrt(lenSq);
            return { x * invLen, y * invLen, z * invLen };
        }
        
        float Distance(const Vector3& other) const { return (*this - other).Length(); }
        float DistanceSq(const Vector3& other) const { return (*this - other).LengthSq(); }
        float Distance2D(const Vector3& other) const { return (*this - other).Length2D(); }
    };
    
    inline float Distance(const Vector3& a, const Vector3& b) { return a.Distance(b); }
    inline float DistanceSq(const Vector3& a, const Vector3& b) { return a.DistanceSq(b); }
    
    inline float Dot(const Vector3& a, const Vector3& b) { return a.Dot(b); }
    inline Vector3 Cross(const Vector3& a, const Vector3& b) { return a.Cross(b); }
    
    // float * Vector3 형태도 지원 (순서 상관 없이 곱하기 가능하도록)
    inline Vector3 operator*(float s, const Vector3& v) { return v * s; }
}

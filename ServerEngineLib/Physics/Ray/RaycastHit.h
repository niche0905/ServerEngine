#pragma once

/*--------------
   RaycastHit
--------------*/
//
// RaycastHit는 Raycast(광선 충돌 검사) 결과를 담는 구조체입니다
// Physics 공용(월드/콜라이더) 결과이며, 전투(Hitbox) 결과는 따로 (Hit::HitResult)로 확장합니다
//

namespace SE::Physics
{
    class Collider;
    
    struct RaycastHit
    {
    public:
        using Vector3 = SE::Math::Vector3;
        
    public:
        bool hit = false;          // 충돌 여부
        
        float t = 0.0f;            // 충돌 파라미터 (Ray의 t 값)
        
        Vector3 point{};          // 충돌 지점 월드 좌표
        Vector3 normal{};         // 충돌 지점 법선 벡터 (단위 벡터)
        
        const Collider* collider = nullptr;     // 충돌한 콜라이더 (nullptr이면 충돌 없음)
        
        void Reset()
        {
            hit = false;
            t = 0.0f;
            point = Vector3{};
            normal = Vector3{};
            collider = nullptr;
        }
    };
    
}
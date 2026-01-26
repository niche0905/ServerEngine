#pragma once
#include "Physics/Collider/Collider.h"
#include "Physics/Collider/OBBCollider.h"

/*---------------
   SwapWrapper
---------------*/
//
// SwapWrapper는 충돌 판정 함수에서 콜라이더의 순서를 바꿔 호출하는 유틸리티 함수입니다
// 구현의 편의를 위해 사용됩니다
//

namespace SE::Physics::Narrowphase
{
    using Vector3 = SE::Math::Vector3;
    using IntersectFn = bool(*)(const Collider&, const Collider&, CollisionResult&);
    
    bool SwapWrapper(const Collider& a, const Collider& b, CollisionResult& out, IntersectFn fn);
    
    inline void ResolveInsideAABBPointNormal(const Vector3& c, const Vector3& boxMin, const Vector3& boxMax, Vector3& outNormal, Vector3& outPoint, float& outFaceDist)
    {
        const float dxMin = SE::Math::Abs(c.x - boxMin.x);
        const float dxMax = SE::Math::Abs(boxMax.x - c.x);
        const float dyMin = SE::Math::Abs(c.y - boxMin.y);
        const float dyMax = SE::Math::Abs(boxMax.y - c.y);
        const float dzMin = SE::Math::Abs(c.z - boxMin.z);
        const float dzMax = SE::Math::Abs(boxMax.z - c.z);
      
        const float dx = SE::Math::Min(dxMin, dxMax);
        const float dy = SE::Math::Min(dyMin, dyMax);
        const float dz = SE::Math::Min(dzMin, dzMax);
      
        outPoint = c;
        outNormal = Vector3{0, 0, 0};
        outFaceDist = 0.0f;
      
        if (dx <= dy and dx <= dz) {
            // X축 면이 가장 가까움
            if (dxMin <= dxMax) {
                outNormal = Vector3{-1, 0, 0};
                outPoint.x = boxMin.x;
                outFaceDist = dxMin;
            }
            else {
                outNormal = Vector3{1, 0, 0};
                outPoint.x = boxMax.x;
                outFaceDist = dxMax;
            }
        }
        else if (dy <= dx and dy <= dz) {
            // Y축 면이 가장 가까움
            if (dyMin <= dyMax) {
                outNormal = Vector3{0, -1, 0};
                outPoint.y = boxMin.y;
                outFaceDist = dyMin;
            }
            else {
                outNormal = Vector3{0, 1, 0};
                outPoint.y = boxMax.y;
                outFaceDist = dyMax;
            }
        }
        else {
            // Z축 면이 가장 가까움
            if (dzMin <= dzMax) {
                outNormal = Vector3{0, 0, -1};
                outPoint.z = boxMin.z;
                outFaceDist = dzMin;
            }
            else {
                outNormal = Vector3{0, 0, 1};
                outPoint.z = boxMax.z;
                outFaceDist = dzMax;
            }
        }
    }
    
    Vector3 ClampPointAABB(const Vector3& p, const Vector3& mn, const Vector3& mx);
    // AABB와 선분의 교차 검사 (슬랩 방법)
    bool IntersectSegmentAABB(const Vector3& A, const Vector3& B, const Vector3& mn, const Vector3& mx, float& outTEnter);
    void ClosestSegmentAABB_AltProj(const Vector3& A, const Vector3& B, const Vector3& mn, const Vector3& mx, float& ioT, Vector3& outP, Vector3& outQ);
    
    inline Vector3 ToLocal_OBB(const Vector3& p, const OBBCollider& O)
    {
        const Vector3 d = p - O.GetCenter();
        return Vector3{
            d.Dot(O.GetAxisX()),
            d.Dot(O.GetAxisY()),
            d.Dot(O.GetAxisZ())
        };
    }
    
    inline Vector3 ToWorld_OBB(const Vector3& localP, const OBBCollider& O)
    {
        return O.GetCenter()
            + O.GetAxisX() * localP.x
            + O.GetAxisY() * localP.y
            + O.GetAxisZ() * localP.z;
    }
    
    inline void ResolveInsideOBBPointNormal(const Vector3& cW, const OBBCollider& O, Vector3& outNormalW, Vector3& outPointW, float& outFaceDist)
    {
        const Vector3 cL = ToLocal_OBB(cW, O);
        
        const Vector3 half = O.GetHalfExtent();
        const Vector3 mnL{-half.x, -half.y, -half.z};
        const Vector3 mxL{ half.x,  half.y,  half.z};
        
        Vector3 nL, pL;
        ResolveInsideAABBPointNormal(cL, mnL, mxL, nL, pL, outFaceDist);
        
        outNormalW = 
            O.GetAxisX() * nL.x +
            O.GetAxisY() * nL.y +
            O.GetAxisZ() * nL.z;
        outNormalW = outNormalW.Normalized(Vector3{0, 1, 0});
        
        outPointW = ToWorld_OBB(pL, O);
    }
    
}

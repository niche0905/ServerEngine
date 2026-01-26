#pragma once
#include "Physics/Collider/Collider.h"

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
    
    void ResolveInsideAABBPointNormal(const Vector3& c, const Vector3& boxMin, const Vector3& boxMax, Vector3& outNormal, Vector3& outPoint, float& outFaceDist);
    
    Vector3 ClampPointAABB(const Vector3& p, const Vector3& mn, const Vector3& mx);
    // AABB와 선분의 교차 검사 (슬랩 방법)
    bool IntersectSegmentAABB(const Vector3& A, const Vector3& B, const Vector3& mn, const Vector3& mx, float& outTEnter);
    void ClosestSegmentAABB_AltProj(const Vector3& A, const Vector3& B, const Vector3& mn, const Vector3& mx, float& ioT, Vector3& outP, Vector3& outQ);
    
}

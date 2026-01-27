#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/OBBCollider.h"
#include "Physics/Collider/CapsuleCollider.h"


namespace SE::Physics::Narrowphase
{
    using Vector3 = SE::Math::Vector3;
   
    bool Intersect_OBB_Capsule(const Collider& a, const Collider& b, CollisionResult& out)
    {
        const auto& O = static_cast<const OBBCollider&>(a);
        const auto& C = static_cast<const CapsuleCollider&>(b);
        
        const float r = C.GetRadius();
        const float rSq = r * r;
        
        const Vector3 half = O.GetHalfExtent();
        const Vector3 mn{-half.x, -half.y, -half.z};
        const Vector3 mx{ half.x,  half.y,  half.z};
        
        const Vector3 A0 = ToLocal_OBB(C.GetPointA(), O);
        const Vector3 B0 = ToLocal_OBB(C.GetPointB(), O);
        
        const Vector3 inflMn = mn - Vector3{r, r, r};
        const Vector3 inflMx = mx + Vector3{r, r, r};
        
        float tEnter = 0.0f;
        if (not IntersectSegmentAABB(A0, B0, inflMn, inflMx, tEnter)) {
            return false;
        }
        
        Vector3 P, Q;   // P: 캡슐 선분 상의 최근접 점, Q: AABB 상의 최근접 점
        float t = tEnter;
        ClosestSegmentAABB_AltProj(A0, B0, mn, mx, t, P, Q);
        
        const Vector3 v = Q - P;
        const float distSq = v.LengthSq();
        
        if (distSq > rSq)
            return false;
      
        out.hit = true;
        
        if (distSq > 1e-12f) {
            const float dist = std::sqrt(distSq);
            
            Vector3 nL = v * (1.0f / dist);
            
            Vector3 nW = OBBLocalDirToWorld(nL, O);
            
            nW = nW.Normalized(Vector3{0, 1, 0});
            
            out.normal = nW;
            out.penetration = r - dist;
            
            out.point = ToWorld_OBB(Q, O);
            return true;
        }
        
        Vector3 nL, pL;
        float faceDist = 0.0f;
        ResolveInsideAABBPointNormal(P, mn, mx, nL, pL, faceDist);
        
        Vector3 nW = OBBLocalDirToWorld(nL, O);
        
        nW = nW.Normalized(Vector3{0, 1, 0});
        
        out.normal = nW;
        out.penetration = r + faceDist;
        out.point = ToWorld_OBB(pL, O);
        
        return true;
    }

    bool Intersect_Capsule_OBB(const Collider& a, const Collider& b, CollisionResult& out)
    {
        return SwapWrapper(a, b, out, &Intersect_OBB_Capsule);
    }
}

#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/OBBCollider.h"
#include "Physics/Collider/SphereCollider.h"


namespace SE::Physics::NarrowPhase
{
    using Vector3 = SE::Math::Vector3;
   
    bool Intersect_OBB_Sphere(const Collider& a, const Collider& b, CollisionResult& out)
    {
        const auto& O = static_cast<const OBBCollider&>(a);
        const auto& S = static_cast<const SphereCollider&>(b);
        
        const Vector3 c = S.GetCenter();
        const float r = S.GetRadius();
        const float rSq = r * r;
        
        const Vector3 p = O.ClosestPoint(c);
        
        const Vector3 d = c - p;
        const float distSq = d.LengthSq();
        
        if (distSq > rSq)
            return false;
        
        out.hit = true;
        
        if (distSq > 1e-12f) {
            const float dist = std::sqrt(distSq);
            out.normal = (p - c) * (1.0f / dist);
            out.penetration = r - dist;
            out.point = p;
            
            return true;
        }
        
        // 구의 중심이 OBB 내부에 있는 경우
        Vector3 nW, pW;
        float faceDist = 0.0f;
        
        ResolveInsideOBBPointNormal(c, O, nW, pW, faceDist);
        
        out.normal = nW;
        out.point = pW;
        out.penetration = r + faceDist;
      
        return true;
    }

    bool Intersect_Sphere_OBB(const Collider& a, const Collider& b, CollisionResult& out)
    {
        return SwapWrapper(a, b, out, &Intersect_OBB_Sphere);
    }
}

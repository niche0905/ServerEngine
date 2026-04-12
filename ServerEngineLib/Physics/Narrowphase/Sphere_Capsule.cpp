#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/SphereCollider.h"
#include "Physics/Collider/CapsuleCollider.h"


namespace SE::Physics::NarrowPhase
{
    using Vector3 = SE::Math::Vector3;
   
    bool Intersect_Sphere_Capsule(const Collider& a, const Collider& b, CollisionResult& out)
    {
        const auto& S = static_cast<const SphereCollider&>(a);
        const auto& C = static_cast<const CapsuleCollider&>(b);
        
        const Vector3 sc = S.GetCenter();
        const float sr = S.GetRadius();
        
        const float cr = C.GetRadius();
        
        const Vector3 q = C.ClosestPointOnSegment(sc);
        
        const Vector3 d = sc - q;
        const float distSq = d.LengthSq();
        
        const float rSum = sr + cr;
        const float rSumSq = rSum * rSum;
        
        if (distSq > rSumSq) {
            return false;
        }
        
        out.hit = true;
        
        if (distSq <= 1e-12f) {
            // out.normal = C.GetAxis();
            out.normal = Vector3{0,1,0};
            out.penetration = rSum;
            out.point = (sc + q) * 0.5f;
            
            return true;
        }
        
        const float dist = std::sqrt(distSq);
        const Vector3 n = d * (1.0f / dist);
        
        out.normal = n;
        out.penetration = rSum - dist;
        
        const Vector3 ps = sc - n * sr;
        const Vector3 pc = q + n * cr;
        out.point = (ps + pc) * 0.5f;
        
        return true;
    }

    bool Intersect_Capsule_Sphere(const Collider& a, const Collider& b, CollisionResult& out)
    {
        return SwapWrapper(a, b, out, &Intersect_Sphere_Capsule);
    }
}

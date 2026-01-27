#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/CapsuleCollider.h"
#include "Physics/Collider/CharacterCapsuleCollider.h"


namespace SE::Physics::Narrowphase
{
    using Vector3 = SE::Math::Vector3;
   
    bool Intersect_Capsule_CharacterCapsule(const Collider& a, const Collider& b, CollisionResult& out)
    {
        const auto& A = static_cast<const CapsuleCollider&>(a);
        const auto& B = static_cast<const CharacterCapsuleCollider&>(b);
      
        const Vector3 A0 = A.GetPointA();
        const Vector3 A1 = A.GetPointB();
        const float ra = A.GetRadius();
      
        const Vector3 B0 = B.GetPointA();
        const Vector3 B1 = B.GetPointB();
        const float rb = B.GetRadius();
      
        Vector3 Pa, Pb;
        float s = 0.0f, t = 0.0f;
        ClosestPtSegmentSegment(A0, A1, B0, B1, s, t , Pa, Pb);
      
        const Vector3 d = Pa - Pb;
        const float distSq = d.LengthSq();
        const float radiusSum = ra + rb;
        const float radiusSumSq = radiusSum * radiusSum;
      
        if (distSq > radiusSumSq)
            return false;
      
        out.hit = true;
      
        if (distSq <= 1e-12f) {
            Vector3 ca = (A0 + A1) * 0.5f;
            Vector3 cb = (B0 + B1) * 0.5f;
            Vector3 n = (ca - cb).Normalized(Vector3{0.0f, 1.0f, 0.0f});
         
            out.normal = n;
            out.penetration = radiusSum;
            out.point = (Pa + Pb) * 0.5f;
         
            return true;
        }
      
        const float dist = std::sqrt(distSq);
        const Vector3 n = d * (1.0f / dist);
      
        out.normal = n;
        out.penetration = radiusSum - dist;
      
        const Vector3 pA = Pa - n * ra;
        const Vector3 pB = Pb + n * rb;
        out.point = (pA + pB) * 0.5f;
      
        return true;
    }

    bool Intersect_CharacterCapsule_Capsule(const Collider& a, const Collider& b, CollisionResult& out)
    {
        return SwapWrapper(a, b, out, &Intersect_Capsule_CharacterCapsule);
    }
}

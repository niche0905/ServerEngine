#include "pch.h"
#include "IntersectUtil.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/CharacterCapsuleCollider.h"

namespace SE::Physics::NarrowPhase
{
    bool Intersect_CharacterCapsule_CharacterCapsule(const Collider& a, const Collider& b, CollisionResult& out)
    {
        using Vector3 = SE::Math::Vector3;
      
        const auto& A = static_cast<const CharacterCapsuleCollider&>(a);
        const auto& B = static_cast<const CharacterCapsuleCollider&>(b);
        
        const float ra = A.GetRadius();
        const float rb = B.GetRadius();
        const float rSum = ra + rb;
        const float rSumSq = rSum * rSum;
        
        const Vector3 A0 = A.GetPointA();
        const Vector3 A1 = A.GetPointB();
        const Vector3 B0 = B.GetPointA();
        const Vector3 B1 = B.GetPointB();
        
        const float dx = A0.x - B0.x;
        const float dz = A0.z - B0.z;
        const float horizSq = dx * dx + dz * dz;
        
        float Ay0 = A0.y, Ay1 = A1.y; if (Ay0 > Ay1) std::swap(Ay0, Ay1);
        float By0 = B0.y, By1 = B1.y; if (By0 > By1) std::swap(By0, By1);
        
        float vert = 0.0f;
        float yA = 0.0f, yB = 0.0f;
        
        if (Ay1 < By0) {
            // B above A
            vert = By0 - Ay1;
            yA = Ay1;
            yB = By0;
        }
        else if (By1 < Ay0) {
            // A above B
            vert = Ay0 - By1;
            yA = Ay0;
            yB = By1;
        }
        else {
            // vertical overlap
            const float yOverlapMin = (Ay0 > By0) ? Ay0 : By0;
            const float yOverlapMax = (Ay1 < By1) ? Ay1 : By1;
            const float yMid = (yOverlapMin + yOverlapMax) * 0.5f;
            vert = 0.0f;
            yA = yMid;
            yB = yMid;
        }
        
        const float distSq = horizSq + (vert * vert);
        if (distSq > rSumSq) {
            return false;
        }
        
        const Vector3 Pa{A0.x, yA, A0.z};
        const Vector3 Pb{B0.x, yB, B0.z};
        
        out.hit = true;
        
        const Vector3 d{ Pa.x - Pb.x, Pa.y - Pb.y, Pa.z - Pb.z };
        
        if (distSq <= 1e-12f) {
            // pivot이 같은 위치에 있는 경우
            
            const float aCy = A.GetCenter().y;
            const float bCy = B.GetCenter().y;
            out.normal = (aCy >= bCy) ? Vector3{0.0f, 1.0f, 0.0f} : Vector3{0.0f, -1.0f, 0.0f};
            out.penetration = rSum;
            out.point = (Pa + Pb) * 0.5f;
            
            return true;
        }
        
        const float dist = std::sqrt(distSq);
        const Vector3 n = d * (1.0f / dist);
        
        out.normal = n;
        out.penetration = rSum - dist;
        
        const Vector3 pA = Pa - n * ra;
        const Vector3 pB = Pb + n * rb;
        out.point = (pA + pB) * 0.5f;
        
        return true;
    }
    
}

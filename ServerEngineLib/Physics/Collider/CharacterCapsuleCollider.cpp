#include "pch.h"
#include "CharacterCapsuleCollider.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"

/*----------------------------
   CharacterCapsuleCollider
----------------------------*/

namespace SE::Physics
{
   static bool RaycastSphereLocal(const Ray& ray,
                               const SE::Math::Vector3& center,
                               float radius,
                               float& outT,
                               SE::Math::Vector3& outNormal)
   {
      using Vector3 = SE::Math::Vector3;

      const Vector3 m = ray.origin - center;
      const float halfB = m.Dot(ray.direction);
      const float c = m.LengthSq() - radius * radius;

      const float disc = halfB * halfB - c;
      if (disc < 0.0f) return false;

      const float sqrtDisc = std::sqrt(disc);

      float t = -halfB - sqrtDisc;              // 가까운 교차점
      if (t < ray.tMin || t > ray.tMax) {
         t = -halfB + sqrtDisc;                // 먼 교차점
         if (t < ray.tMin || t > ray.tMax)
            return false;
      }

      outT = t;
      const Vector3 p = ray.At(t);
      outNormal = (p - center).Normalized(Vector3(0,1,0));
      return true;
   }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
   
   CharacterCapsuleCollider::CharacterCapsuleCollider(const Vector3& base, float height, float radius)
   {
      Set(base, height, radius);
   }

   ColliderType CharacterCapsuleCollider::GetType() const
   {
      return ColliderType::CharacterCapsule;
   }

   Collider* CharacterCapsuleCollider::Clone() const
   {
      return new CharacterCapsuleCollider(*this);
   }

   void CharacterCapsuleCollider::UpdateWorld(const Math::Vector3& position, float yaw)
   {
      worldBase_ = position + Math::RotateYaw(localBase_, yaw);
      worldHeight_ = localHeight_;
      worldRadius_ = localRadius_;
      
      RecalcWorldAABB();
   }

   void CharacterCapsuleCollider::Set(const Vector3& base, float height, float radius)
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(radius >= 0.0f && "Character Capsule radius must be >= 0");
         assert(height >= 2.0f * radius && "Character Capsule height must be >= 2*radius");
      }
      
      localBase_ = base;
      localHeight_ = height;
      localRadius_ = SE::Math::Abs(radius);
      
      if (localHeight_ < 2.0f * localRadius_) localHeight_ = 2.0f * localRadius_;  // 캡슐 크기 유효하게
      
      UpdateWorld(Vector3{0.0f, 0.0f, 0.0f}, 0.0f);
   }

   const AABBCollider& CharacterCapsuleCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   bool CharacterCapsuleCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(SE::Math::NearlyZero(ray.direction.LengthSq() - 1.0f, 1e-3f) && "Ray direction must be normalized");
      }
      
      const float r = worldRadius_;
      if (r <= 0.0f)
         return false;
      
      const float yA = worldBase_.y + r;
      const float yB = worldBase_.y + (worldHeight_ - r);
      
      const Vector3 capA{ worldBase_.x, yA, worldBase_.z };
      const Vector3 capB{ worldBase_.x, yB, worldBase_.z };
      
      bool hit = false;
      float bestT = ray.tMax;
      Vector3 bestN{0, 0, 0};
      
      const float ox = ray.origin.x - worldBase_.x;
      const float oz = ray.origin.z - worldBase_.z;
      const float dx = ray.direction.x;
      const float dz = ray.direction.z;
      
      const float a = dx * dx + dz * dz;
      
      if (a > 1e-12f) {
         const float b = 2.0f * (ox * dx + oz * dz);
         const float c = (ox * ox + oz * oz) - r * r;
         
         const float disc = b * b - 4.0f * a * c;
         if (disc >= 0.0f) {
            const float sqrtDisc = std::sqrt(disc);
            float t0 = (-b - sqrtDisc) / (2.0f * a);
            float t1 = (-b + sqrtDisc) / (2.0f * a);
            if (t0 > t1) std::swap(t0, t1);
            
            auto tryBody = [&](float tCand)
            {
               if (tCand < ray.tMin or tCand > ray.tMax)
                  return;
               
               const float y = ray.origin.y + ray.direction.y * tCand;
               if (y < yA or y > yB)
                  return;  // 실린더 몸통 밖
               
               const Vector3 p = ray.At(tCand);
               
               Vector3 n{p.x-worldBase_.x, 0.0f, p.z - worldBase_.z};
               n = n.Normalized(Vector3{1.0f, 0.0f, 0.0f});
               
               if (!hit or tCand < bestT) {
                  hit = true;
                  bestT = tCand;
                  bestN = n;
               }
            };
            
            tryBody(t0);
            tryBody(t1);
         }
      }
      
      // 캡슐 머리, 꼬리 구면 검사
      {
         float tS;
         Vector3 nS;
         
         if (RaycastSphereLocal(ray, capA, r, tS, nS)) {
            if (!hit || tS < bestT) {
               hit = true;
               bestT = tS;
               bestN = nS;
            }
         }
         
         if (RaycastSphereLocal(ray, capB, r, tS, nS)) {
            if (!hit || tS < bestT) {
               hit = true;
               bestT = tS;
               bestN = nS;
            }
         }
      }
      
      if (not hit)
         return false;
      
      out.hit = true;
      out.t = bestT;
      out.point = ray.At(bestT);
      out.normal = bestN;
      out.collider = this;
      
      return true;
   }

   SE::Math::Vector3 CharacterCapsuleCollider::ClosestPointOnSegment(const Vector3& point) const
   {
      const float yA = worldBase_.y + worldRadius_;
      const float yB = worldBase_.y + (worldHeight_ - worldRadius_);
      
      float y = point.y;
      if (y < yA) y = yA;
      if (y > yB) y = yB;
      
      return Vector3{ worldBase_.x, y, worldBase_.z };
   }

   SE::Math::Vector3 CharacterCapsuleCollider::ClosestPoint(const Vector3& point) const
   {
      const Vector3 c = ClosestPointOnSegment(point);
      
      Vector3 v = point - c;
      const float distSq = v.LengthSq();
      
      // point가 중심선(Seqment) 위의 점이라면 가까운 표면을 정할 수 없다
      if (distSq <= 1e-12f)
         return c;
      
      const float rSq = worldRadius_ * worldRadius_;
      if (distSq <= rSq)
         return point;  // 캡슐 내부라면 그대로 반환
      
      const Vector3 n = v.Normalized();
      return c + (n * worldRadius_);
   }

   float CharacterCapsuleCollider::DistanceSqToSegment(const Vector3& point) const
   {
      const Vector3 c = ClosestPointOnSegment(point);
      return (point - c).LengthSq();
   }

   void CharacterCapsuleCollider::RecalcWorldAABB()
   {
      const Vector3 mn{
         worldBase_.x - worldRadius_,
         worldBase_.y,
         worldBase_.z - worldRadius_
     };

      const Vector3 mx{
         worldBase_.x + worldRadius_,
         worldBase_.y + worldHeight_,
         worldBase_.z + worldRadius_
     };

      worldAABB_.SetMinMax(mn, mx);
   }
}

#include "pch.h"
#include "CapsuleCollider.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"

/*-------------------
   CapsuleCollider
-------------------*/

namespace SE::Physics
{
   static bool RaycastSphere(const Ray& ray, const SE::Math::Vector3& center, float radius, float& outT, SE::Math::Vector3& outNormal)
   {
      using Vector3 = SE::Math::Vector3;
      
      const Vector3 m = ray.origin - center;
      const float halfB = m.Dot(ray.direction);
      const float c = m.LengthSq() - radius * radius;
      
      const float disc = halfB * halfB - c;
      if (disc < 0.0f)
         return false;
      
      const float sqrtDisc = std::sqrt(disc);
      
      float t = -halfB - sqrtDisc;
      if (t < ray.tMin or t > ray.tMax) {
         t = -halfB + sqrtDisc;
         if (t < ray.tMin or t > ray.tMax)
            // 교차 없음 (유효하지  않은교차점)         
            return false;
      }
      
      outT = t;
      const Vector3 p = ray.At(t);
      outNormal = (p - center).Normalized(Vector3(0, 1, 0));
      
      return true;
   }
   
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
   
   CapsuleCollider::CapsuleCollider(const Vector3& pointA, const Vector3& pointB, float radius)
   {
      Set(pointA, pointB, radius);
   }

   ColliderType CapsuleCollider::GetType() const
   {
      return ColliderType::Capsule;
   }

   Collider* CapsuleCollider::Clone() const
   {
      return new CapsuleCollider(*this);
   }

   void CapsuleCollider::UpdateWorld(const Math::Vector3& position, float yaw)
   {
      worldPointA_ = position + Math::RotateYaw(localPointA_, yaw);
      worldPointB_ = position + Math::RotateYaw(localPointB_, yaw);
      worldRadius_ = localRadius_;
      
      RecalcDerived();
      RecalcWorldAABB();
   }

   void CapsuleCollider::Set(const Vector3& pointA, const Vector3& pointB, float radius)
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(radius >= 0.0f && "Capsule Radius must be >= 0");
      }
      
      localPointA_ = pointA;
      localPointB_ = pointB;
      localRadius_ = SE::Math::Abs(radius);
      
      UpdateWorld(Vector3{0.0f, 0.0f, 0.0f}, 0.0f);
   }

   bool CapsuleCollider::ContainsPoint(const Math::Vector3& point) const
   {
      return DistanceSqToSegment(point) <= RadiusSq();
   }

   bool CapsuleCollider::ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest,
      Math::Vector3& outNormal) const
   {
      constexpr float Epsilon = 1e-6f;

      const Vector3 centerOnSegment = ClosestPointOnSegment(point);
      Vector3 dir = point - centerOnSegment;

      const float lenSq = dir.LengthSq();

      // point가 캡슐 중심선 위에 거의 있는 경우
      if (lenSq <= Epsilon)
      {
         // 방향을 특정할 수 없으므로 캡슐 축과 수직인 임의 방향 사용
         Vector3 fallbackNormal = Vector3{1.0f, 0.0f, 0.0f};

         // fallbackNormal이 캡슐 축과 거의 평행하면 다른 축 사용
         if (std::abs(fallbackNormal.Dot(dir_)) > 0.9f)
            fallbackNormal = Vector3{0.0f, 1.0f, 0.0f};

         // 캡슐 축 성분 제거해서 축에 수직인 방향 생성
         fallbackNormal = fallbackNormal - dir_ * fallbackNormal.Dot(dir_);
         fallbackNormal = fallbackNormal.Normalized(Vector3{1.0f, 0.0f, 0.0f});

         outNormal = fallbackNormal;
         outClosest = centerOnSegment + outNormal * worldRadius_;

         return true;
      }

      outNormal = dir.Normalized();
      outClosest = centerOnSegment + outNormal * worldRadius_;

      return true;
   }

   const AABBCollider& CapsuleCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   bool CapsuleCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(SE::Math::NearlyZero(ray.direction.LengthSq() - 1.0f, 1e-3f) && "Ray direction must be normalized");
      }
      
      const Vector3 A = worldPointA_;
      const Vector3 B = worldPointB_;
      const float r = worldRadius_;
      
      const Vector3 AB = B - A;
      const float lenSq = AB.LengthSq();
      
      // Sphere와 다를게 없다 (A==B)
      if (lenSq <= 1e-12f) {
         float tS;
         Vector3 nS;
         if (not RaycastSphere(ray, A, r, tS, nS))
            return false;
         
         out.hit = true;
         out.t = tS;
         out.point = ray.At(tS);
         out.normal = nS;
         out.collider = this;
         return true;
      }
      
      const float len = std::sqrt(lenSq);
      const Vector3 u = AB * (1.0f / len);   // 단위벡터
      
      const Vector3 p = ray.origin - A;
      
      const float pAxis = p.Dot(u);
      const float dAxis = ray.direction.Dot(u);
      
      const Vector3 pPerp = p - (u * pAxis);
      const Vector3 dPerp = ray.direction - (u * dAxis);
      
      // 후보 중 가장 가까운 hit 찾기
      bool hit = false;
      float bestT = ray.tMax;
      Vector3 bestN{0, 0, 0};
      
      const float a = dPerp.LengthSq();
      
      if (not SE::Math::NearlyZero(a, 1e-12f)) {
         
         const float b = 2.0f * pPerp.Dot(dPerp);
         const float c = pPerp.LengthSq() - r * r;
         
         const float disc = b * b - 4.0f * a * c;
         if (disc >= 0.0f) {
            const float sqrtDisc = std::sqrt(disc);
            float t0 = (-b - sqrtDisc) / (2.0f * a);
            float t1 = (-b + sqrtDisc) / (2.0f * a);
            if (t0 > t1) std::swap(t0, t1);
            
            auto tryCylinderT = [&](float tCand)
            {
               if (tCand < ray.tMin or tCand > ray.tMax) return;
               const float s = pAxis + tCand * dAxis;
               if (s < 0.0f or s > len) return; // 선분 범위 박 (몸통 아님)
               
               // 몸통 normal: 축에 수직인 방향
               const Vector3 n = (pPerp + dPerp * tCand).Normalized(Vector3(0,1,0));
               if (not hit or tCand < bestT) {
                  hit = true;
                  bestT = tCand;
                  bestN = n;
               }
            };
            
            // 가까운 교차점부터 시도
            tryCylinderT(t0);
            tryCylinderT(t1);
         }
      }
      else {
         // Ray가 캡슐 축과 평행한 경우
         
         const float distSq = pPerp.LengthSq();
         if (distSq <= r * r) {
            if (not SE::Math::NearlyZero(dAxis, 1e-12f)) {
               float tEnter = (0.0f - pAxis) / dAxis;
               float tExit = (len - pAxis) / dAxis;
               if (tEnter > tExit) std::swap(tEnter, tExit);
               
               float tCand = tEnter;
               if (tCand < ray.tMin or tCand > ray.tMax) {
                  tCand = tExit;
               }
               
               if (tCand >= ray.tMin and tCand <= ray.tMax) {
                  const Vector3 hitP = ray.At(tCand);
                  float s = pAxis + tCand * dAxis;
                  s = SE::Math::Clamp(s, 0.0f, len);
                  const Vector3 Q = A + u * s;
                  const Vector3 n = (hitP - Q).Normalized(Vector3(0,1,0));
                  
                  hit = true;
                  bestT = tCand;
                  bestN = n;
               }
            }
         }
      }
      
      {
         float tS;
         Vector3 nS;
         
         if (RaycastSphere(ray, A, r, tS, nS)) {
            if (not hit or tS < bestT) {
               hit = true;
               bestT = tS;
               bestN = nS;
            }
         }
         
         if (RaycastSphere(ray, B, r, tS, nS)) {
            if (not hit or tS < bestT) {
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

   bool CapsuleCollider::SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius,
      RaycastHit& outHit) const
   {
      if (radius < 0.0f)
      return false;

      const Vector3 delta = to - from;
      const float dist = delta.Length();

      if (dist <= 1e-4f)
         return false;

      Ray ray;
      ray.origin = from;
      ray.direction = delta.Normalized();
      ray.tMin = 0.0f;
      ray.tMax = dist;

      assert(SE::Math::NearlyZero(ray.direction.LengthSq() - 1.0f, 1e-3f) && "Ray direction must be normalized");

      const Vector3 A = worldPointA_;
      const Vector3 B = worldPointB_;
      const float r = worldRadius_ + radius;

      if (r <= 0.0f)
         return false;

      const Vector3 AB = B - A;
      const float lenSq = AB.LengthSq();

      if (lenSq <= 1e-12f) {
         float tS;
         Vector3 nS;

         if (!RaycastSphere(ray, A, r, tS, nS))
            return false;

         outHit.hit = true;
         outHit.t = tS;
         outHit.point = ray.At(tS) - nS * radius;
         outHit.normal = nS;
         outHit.collider = this;

         return true;
      }

      const float len = std::sqrt(lenSq);
      const Vector3 u = AB * (1.0f / len);

      const Vector3 p = ray.origin - A;

      const float pAxis = p.Dot(u);
      const float dAxis = ray.direction.Dot(u);

      const Vector3 pPerp = p - (u * pAxis);
      const Vector3 dPerp = ray.direction - (u * dAxis);

      bool hit = false;
      float bestT = ray.tMax;
      Vector3 bestN{0, 0, 0};

      const float a = dPerp.LengthSq();

      if (!SE::Math::NearlyZero(a, 1e-12f)) {
         const float b = 2.0f * pPerp.Dot(dPerp);
         const float c = pPerp.LengthSq() - r * r;

         const float disc = b * b - 4.0f * a * c;
         if (disc >= 0.0f) {
            const float sqrtDisc = std::sqrt(disc);
            float t0 = (-b - sqrtDisc) / (2.0f * a);
            float t1 = (-b + sqrtDisc) / (2.0f * a);
            if (t0 > t1) std::swap(t0, t1);

            auto tryCylinderT = [&](float tCand)
            {
               if (tCand < ray.tMin || tCand > ray.tMax)
                  return;

               const float s = pAxis + tCand * dAxis;
               if (s < 0.0f || s > len)
                  return;

               const Vector3 n = (pPerp + dPerp * tCand).Normalized(Vector3{0, 1, 0});

               if (!hit || tCand < bestT) {
                  hit = true;
                  bestT = tCand;
                  bestN = n;
               }
            };

            tryCylinderT(t0);
            tryCylinderT(t1);
         }
      }
      else {
         const float distSq = pPerp.LengthSq();

         if (distSq <= r * r) {
            if (!SE::Math::NearlyZero(dAxis, 1e-12f)) {
               float tEnter = (0.0f - pAxis) / dAxis;
               float tExit = (len - pAxis) / dAxis;

               if (tEnter > tExit)
                  std::swap(tEnter, tExit);

               float tCand = tEnter;

               if (tCand < ray.tMin || tCand > ray.tMax)
                  tCand = tExit;

               if (tCand >= ray.tMin && tCand <= ray.tMax) {
                  const Vector3 hitP = ray.At(tCand);

                  float s = pAxis + tCand * dAxis;
                  s = SE::Math::Clamp(s, 0.0f, len);

                  const Vector3 Q = A + u * s;
                  const Vector3 n = (hitP - Q).Normalized(Vector3{0, 1, 0});

                  hit = true;
                  bestT = tCand;
                  bestN = n;
               }
            }
         }
      }

      {
         float tS;
         Vector3 nS;

         if (RaycastSphere(ray, A, r, tS, nS)) {
            if (!hit || tS < bestT) {
               hit = true;
               bestT = tS;
               bestN = nS;
            }
         }

         if (RaycastSphere(ray, B, r, tS, nS)) {
            if (!hit || tS < bestT) {
               hit = true;
               bestT = tS;
               bestN = nS;
            }
         }
      }

      if (!hit)
         return false;

      outHit.hit = true;
      outHit.t = bestT;
      outHit.point = ray.At(bestT);
      outHit.normal = bestN;
      outHit.collider = this;

      return true;
   }

   SE::Math::Vector3 CapsuleCollider::ClosestPointOnSegment(const Vector3& point) const
   {
      const Vector3 ab = worldPointB_ - worldPointA_;
      const float abLenSq = ab.LengthSq();

      if (abLenSq <= 1e-12f)
         return worldPointA_; // A==B면 그냥 그 점

      const float t = SE::Math::Clamp((point - worldPointA_).Dot(ab) / abLenSq, 0.0f, 1.0f);
      return worldPointA_ + (ab * t);
   }

   SE::Math::Vector3 CapsuleCollider::ClosestPoint(const Vector3& point) const
   {
      const Vector3 c = ClosestPointOnSegment(point);   // 중심선 위 최근접점
      const Vector3 v = point - c;
      const float distSq = v.LengthSq();

      // 중심선 위면 방향이 없으니 그냥 c에서 반지름만큼 임의 방향으로 못 밀어냄
      // inside 처리로 point 반환이 더 안전
      if (distSq <= 1e-12f)
         return c;

      const float rSq = RadiusSq();
      if (distSq <= rSq)
         return point; // 캡슐 내부면 점 그대로(서버 판정에서 자주 쓰는 정책)

      const Vector3 n = v.Normalized();     // c->point 방향 단위벡터
      return c + (n * worldRadius_);  
   }

   float CapsuleCollider::DistanceSqToSegment(const Vector3& point) const
   {
      const Vector3 c = ClosestPointOnSegment(point);
      return (point - c).LengthSq();
   }

   void CapsuleCollider::RecalcWorldAABB()
   {
      Vector3 mn{
         SE::Math::Min(worldPointA_.x, worldPointB_.x),
         SE::Math::Min(worldPointA_.y, worldPointB_.y),
         SE::Math::Min(worldPointA_.z, worldPointB_.z)
     };

      Vector3 mx{
         SE::Math::Max(worldPointA_.x, worldPointB_.x),
         SE::Math::Max(worldPointA_.y, worldPointB_.y),
         SE::Math::Max(worldPointA_.z, worldPointB_.z)
     };

      const Vector3 r{ worldRadius_, worldRadius_, worldRadius_ };
      mn = mn - r;
      mx = mx + r;

      worldAABB_.SetMinMax(mn, mx);
   }

   void CapsuleCollider::RecalcDerived()
   {
      Vector3 ab = worldPointB_ - worldPointA_;
      float lenSq = ab.LengthSq();
      if (lenSq <= 1e-12f) {
         dir_ = Vector3{0,0,1};
         halfLen_ = 0.0f;
      }
      else {
         float len = std::sqrt(lenSq);
         dir_ = ab * (1.0f / len);
         halfLen_ = len * 0.5f;
      }
   }
}

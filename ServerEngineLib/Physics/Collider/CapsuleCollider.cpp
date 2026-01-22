#include "pch.h"
#include "CapsuleCollider.h"

/*-------------------
   CapsuleCollider
-------------------*/

namespace SE::Physics
{
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

   bool CapsuleCollider::Intersect(const Collider& other, CollisionResult& out) const
   {
      // TODO: 충돌 판정
      
      return false;
   }

   void CapsuleCollider::Set(const Vector3& pointA, const Vector3& pointB, float radius)
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(radius >= 0.0f && "Capsule Radius must be > 0");
      }
      
      pointA_ = pointA;
      pointB_ = pointB;
      radius_ = SE::Math::Abs(radius);
      
      RecalcDerived();
      RecalcWorldAABB();
   }

   const AABBCollider& CapsuleCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   bool CapsuleCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      return false;
   }

   Vector3 CapsuleCollider::ClosestPointOnSegment(const Vector3& point) const
   {
      const Vector3 ab = pointB_ - pointA_;
      const float abLenSq = ab.LengthSq();

      if (abLenSq <= 1e-12f)
         return pointA_; // A==B면 그냥 그 점

      const float t = SE::Math::Clamp((point - pointA_).Dot(ab) / abLenSq, 0.0f, 1.0f);
      return pointA_ + (ab * t);
   }

   Vector3 CapsuleCollider::ClosestPoint(const Vector3& point) const
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
      return c + (n * radius_);  
   }

   float CapsuleCollider::DistanceSqToSegment(const Vector3& point) const
   {
      const Vector3 c = ClosestPointOnSegment(point);
      return (point - c).LengthSq();
   }

   void CapsuleCollider::RecalcWorldAABB()
   {
      Vector3 mn{
         SE::Math::Min(pointA_.x, pointB_.x),
         SE::Math::Min(pointA_.y, pointB_.y),
         SE::Math::Min(pointA_.z, pointB_.z)
     };

      Vector3 mx{
         SE::Math::Max(pointA_.x, pointB_.x),
         SE::Math::Max(pointA_.y, pointB_.y),
         SE::Math::Max(pointA_.z, pointB_.z)
     };

      const Vector3 r{ radius_, radius_, radius_ };
      mn = mn - r;
      mx = mx + r;

      worldAABB_.SetMinMax(mn, mx);
   }

   void CapsuleCollider::RecalcDerived()
   {
      Vector3 ab = pointB_ - pointA_;
      float lenSq = ab.LengthSq();
      if (lenSq <= 1e-12f) {
         dir_ = Vector3{1,0,0};
         halfLen_ = 0.0f;
      }
      else {
         float len = std::sqrt(lenSq);
         dir_ = ab * (1.0f / len);
         halfLen_ = len * 0.5f;
      }
   }
}

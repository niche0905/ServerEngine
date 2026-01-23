#include "pch.h"
#include "OBBCollider.h"

#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"

/*---------------
   OBBCollider
---------------*/

namespace SE::Physics
{
   OBBCollider::OBBCollider(const Vector3& center, const Vector3& halfExtent, 
      const Vector3& axisX, const Vector3& axisY, const Vector3& axisZ)
   {
      Set(center, halfExtent, axisX, axisY, axisZ);
   }

   ColliderType OBBCollider::GetType() const
   {
      return ColliderType::OBB;
   }

   Collider* OBBCollider::Clone() const
   {
      return new OBBCollider(*this);
   }

   bool OBBCollider::Intersect(const Collider& other, CollisionResult& out) const
   {
      // TODO: 충돌체 끼리 충돌 검사 방법 찾기 Switch Case는 별로인거 같다
      
      return false;
   }

   void OBBCollider::Set(const Vector3& center, const Vector3& halfExtent, 
      const Vector3& axisX, const Vector3& axisY, const Vector3& axisZ)
   {
      center_ = center;
      
      half_.x = SE::Math::Abs(halfExtent.x);
      half_.y = SE::Math::Abs(halfExtent.y);
      half_.z = SE::Math::Abs(halfExtent.z);
      
      axis_[0] = axisX.Normalized(Vector3{1, 0, 0});
      axis_[1] = axisY.Normalized(Vector3{0, 1, 0});
      axis_[2] = axisZ.Normalized(Vector3{0, 0, 1});
      
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(SE::Math::NearlyZero(axis_[0].Dot(axis_[1])) && "OBB axes must be orthogonal");
         assert(SE::Math::NearlyZero(axis_[0].Dot(axis_[2])) && "OBB axes must be orthogonal");
         assert(SE::Math::NearlyZero(axis_[1].Dot(axis_[2])) && "OBB axes must be orthogonal");
      }
      
      RecalcWorldAABB();
   }

   const AABBCollider& OBBCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   bool OBBCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(SE::Math::NearlyZero(axis_[0].LengthSq() - 1.0f, 1e-3f) && "OBB axisX must be normalized");
         assert(SE::Math::NearlyZero(axis_[1].LengthSq() - 1.0f, 1e-3f) && "OBB axisY must be normalized");
         assert(SE::Math::NearlyZero(axis_[2].LengthSq() - 1.0f, 1e-3f) && "OBB axisZ must be normalized");
      }
      
      const Vector3 o = ray.origin - center_;
      
      const Vector3 oL{
         o.Dot(axis_[0]),
         o.Dot(axis_[1]),
         o.Dot(axis_[2])
      };
      
      const Vector3 dL{
         ray.direction.Dot(axis_[0]),
         ray.direction.Dot(axis_[1]),
         ray.direction.Dot(axis_[2])
      };
      
      const Vector3 mn = Vector3{-half_.x, -half_.y, -half_.z};
      const Vector3 mx = Vector3{ half_.x,  half_.y,  half_.z};
      
      float tMin = ray.tMin;
      float tMax = ray.tMax;
      
      Vector3 enterN{0.0f, 0.0f, 0.0f};
      Vector3 exitN{0.0f, 0.0f, 0.0f};
      
      auto slab = [&](float origin, float dir, float minB, float maxB, const Vector3& nEnter, const Vector3& nExit) -> bool
      {
         if (std::fabs(dir) <= 1e-12f) {
            return (origin >= minB and origin <= maxB);
         }
         
         const float invD = 1.0f / dir;
         float t1 = (minB - origin) * invD;
         float t2 = (maxB - origin) * invD;
         
         Vector3 n1 = nEnter;
         Vector3 n2 = nExit;
         
         if (t1 > t2) {
            std::swap(t1, t2);
            std::swap(n1, n2);
         }
         
         if (t1 > tMin) {
            tMin = t1;
            enterN = n1;
         }
         
         if (t2 < tMax) {
            tMax = t2;
            exitN = n2;
         }
         
         return (tMin <= tMax);
      };
      
      if (not slab(oL.x, dL.x, mn.x, mx.x, Vector3{-1, 0, 0}, Vector3{ 1, 0, 0})) return false;
      if (not slab(oL.y, dL.y, mn.y, mx.y, Vector3{ 0,-1, 0}, Vector3{ 0, 1, 0})) return false;
      if (not slab(oL.z, dL.z, mn.z, mx.z, Vector3{ 0, 0,-1}, Vector3{ 0, 0, 1})) return false;
      
      float tHit = tMin;
      Vector3 nL = enterN;
      
      if (tHit < ray.tMin) {
         tHit = tMax;
         nL = exitN;
         if (nL.LengthSq() < 1e-12f)
            nL = enterN;   // 극단적 상황 대비 (최소 fallback)
      }
      
      if (tHit < ray.tMin or tHit > ray.tMax)
         return false;
      
      out.hit = true;
      out.t = tHit;
      out.point = ray.At(tHit);
      
      Vector3 nW = axis_[0] * nL.x + axis_[1] * nL.y + axis_[2] * nL.z;
      out.normal = nW;  // axis_가 정규 직교면 nW도 정규 벡터
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         // 만약을 위해 정규화 확인
         assert(SE::Math::NearlyZero(nW.LengthSq() - 1.0f, 1e-3f) && "OBB Ray Normal Vector must be normalized");
      }
      out.collider = this;
      
      return true;
   }

   SE::Math::Vector3 OBBCollider::ClosestPoint(const Vector3& point) const
   {
      const Vector3 d = point - center_;
      
      Vector3 q = center_;
      
      float dist = d.Dot(axis_[0]);
      dist = SE::Math::Max(-half_.x, SE::Math::Min(dist , half_.x));
      q = q + (axis_[0] * dist);
      
      dist = d.Dot(axis_[1]);
      dist = SE::Math::Max(-half_.y, SE::Math::Min(dist , half_.y));
      q = q + (axis_[1] * dist);
      
      dist = d.Dot(axis_[2]);
      dist = SE::Math::Max(-half_.z, SE::Math::Min(dist , half_.z));
      q = q + (axis_[2] * dist);
      
      return q;
   }

   void OBBCollider::RecalcWorldAABB()
   {
      Vector3 r;
      r.x = SE::Math::Abs(axis_[0].x) * half_.x + SE::Math::Abs(axis_[1].x) * half_.y + SE::Math::Abs(axis_[2].x) * half_.z;
      r.y = SE::Math::Abs(axis_[0].y) * half_.x + SE::Math::Abs(axis_[1].y) * half_.y + SE::Math::Abs(axis_[2].y) * half_.z;
      r.z = SE::Math::Abs(axis_[0].z) * half_.x + SE::Math::Abs(axis_[1].z) * half_.y + SE::Math::Abs(axis_[2].z) * half_.z;
      
      const Vector3 mn = center_ - r;
      const Vector3 mx = center_ + r;
      
      worldAABB_.SetMinMax(mn, mx);
   }
}

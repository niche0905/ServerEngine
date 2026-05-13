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

   void OBBCollider::UpdateWorld(const Math::Vector3& position, float yaw)
   {
      worldCenter_ = position + Math::RotateYaw(localCenter_, yaw);
      worldHalf_ = localHalf_;
      
      worldAxis_[0] = Math::RotateYaw(localAxis_[0], yaw).Normalized(Vector3{1, 0, 0});
      worldAxis_[1] = Math::RotateYaw(localAxis_[1], yaw).Normalized(Vector3{0, 1, 0});
      worldAxis_[2] = Math::RotateYaw(localAxis_[2], yaw).Normalized(Vector3{0, 0, 1});
      
      RecalcWorldAABB();
   }

   void OBBCollider::Set(const Vector3& center, const Vector3& halfExtent, 
                         const Vector3& axisX, const Vector3& axisY, const Vector3& axisZ)
   {
      localCenter_ = center;
      
      localHalf_.x = SE::Math::Abs(halfExtent.x);
      localHalf_.y = SE::Math::Abs(halfExtent.y);
      localHalf_.z = SE::Math::Abs(halfExtent.z);
      
      localAxis_[0] = axisX.Normalized(Vector3{1, 0, 0});
      localAxis_[1] = axisY.Normalized(Vector3{0, 1, 0});
      localAxis_[2] = axisZ.Normalized(Vector3{0, 0, 1});
      
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(SE::Math::NearlyZero(localAxis_[0].Dot(localAxis_[1])) && "OBB axes must be orthogonal");
         assert(SE::Math::NearlyZero(localAxis_[0].Dot(localAxis_[2])) && "OBB axes must be orthogonal");
         assert(SE::Math::NearlyZero(localAxis_[1].Dot(localAxis_[2])) && "OBB axes must be orthogonal");
      }
      
      UpdateWorld(Vector3{0.0f, 0.0f, 0.0f}, 0.0f);
   }

   const AABBCollider& OBBCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   bool OBBCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(SE::Math::NearlyZero(worldAxis_[0].LengthSq() - 1.0f, 1e-3f) && "OBB axisX must be normalized");
         assert(SE::Math::NearlyZero(worldAxis_[1].LengthSq() - 1.0f, 1e-3f) && "OBB axisY must be normalized");
         assert(SE::Math::NearlyZero(worldAxis_[2].LengthSq() - 1.0f, 1e-3f) && "OBB axisZ must be normalized");
      }
      
      const Vector3 o = ray.origin - worldCenter_;
      
      const Vector3 oL{
         o.Dot(worldAxis_[0]),
         o.Dot(worldAxis_[1]),
         o.Dot(worldAxis_[2])
      };
      
      const Vector3 dL{
         ray.direction.Dot(worldAxis_[0]),
         ray.direction.Dot(worldAxis_[1]),
         ray.direction.Dot(worldAxis_[2])
      };
      
      const Vector3 mn = Vector3{-worldHalf_.x, -worldHalf_.y, -worldHalf_.z};
      const Vector3 mx = Vector3{ worldHalf_.x,  worldHalf_.y,  worldHalf_.z};
      
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
      
      if (nL.LengthSq() < 1e-12f) {
         tHit = tMax;
         nL = exitN;
      }
      else if (tHit < ray.tMin) {
         tHit = tMax;
         nL = exitN;
      }
      
      if (tHit < ray.tMin or tHit > ray.tMax)
         return false;

      if (nL.LengthSq() < 1e-12f)
         return false;
      
      out.hit = true;
      out.t = tHit;
      out.point = ray.At(tHit);
      
      Vector3 nW = worldAxis_[0] * nL.x + worldAxis_[1] * nL.y + worldAxis_[2] * nL.z;
      nW = nW.Normalized();
      out.normal = nW;  // axis_가 정규 직교면 nW도 정규 벡터
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         // 만약을 위해 정규화 확인
         assert(SE::Math::NearlyZero(nW.LengthSq() - 1.0f, 1e-3f) && "OBB Ray Normal Vector must be normalized");
      }
      out.collider = this;
      
      return true;
   }

   bool OBBCollider::SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius,
      RaycastHit& outHit) const
   {
      assert(SE::Math::NearlyZero(worldAxis_[0].LengthSq() - 1.0f, 1e-3f) && "OBB axisX must be normalized");
      assert(SE::Math::NearlyZero(worldAxis_[1].LengthSq() - 1.0f, 1e-3f) && "OBB axisY must be normalized");
      assert(SE::Math::NearlyZero(worldAxis_[2].LengthSq() - 1.0f, 1e-3f) && "OBB axisZ must be normalized");

      if (radius < 0.0f)
         return false;

      const Vector3 delta = to - from;
      const float dist = delta.Length();

      if (dist <= 1e-4f)
         return false;

      const Vector3 dir = delta.Normalized();

      Ray ray;
      ray.origin = from;
      ray.direction = dir;
      ray.tMin = 0.0f;
      ray.tMax = dist;

      const Vector3 o = ray.origin - worldCenter_;

      const Vector3 oL{
         o.Dot(worldAxis_[0]),
         o.Dot(worldAxis_[1]),
         o.Dot(worldAxis_[2])
      };

      const Vector3 dL{
         ray.direction.Dot(worldAxis_[0]),
         ray.direction.Dot(worldAxis_[1]),
         ray.direction.Dot(worldAxis_[2])
      };

      const Vector3 expandedHalf = worldHalf_ + Vector3{ radius, radius, radius };

      const Vector3 mn = Vector3{ -expandedHalf.x, -expandedHalf.y, -expandedHalf.z };
      const Vector3 mx = Vector3{  expandedHalf.x,  expandedHalf.y,  expandedHalf.z };

      float tMin = ray.tMin;
      float tMax = ray.tMax;

      Vector3 enterN{ 0.0f, 0.0f, 0.0f };
      Vector3 exitN{ 0.0f, 0.0f, 0.0f };

      auto slab = [&](float origin, float dir, float minB, float maxB,
                      const Vector3& nEnter, const Vector3& nExit) -> bool
      {
         if (std::fabs(dir) <= 1e-12f) {
            return origin >= minB && origin <= maxB;
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

         return tMin <= tMax;
      };

      if (!slab(oL.x, dL.x, mn.x, mx.x, Vector3{ -1,  0,  0 }, Vector3{  1,  0,  0 })) return false;
      if (!slab(oL.y, dL.y, mn.y, mx.y, Vector3{  0, -1,  0 }, Vector3{  0,  1,  0 })) return false;
      if (!slab(oL.z, dL.z, mn.z, mx.z, Vector3{  0,  0, -1 }, Vector3{  0,  0,  1 })) return false;

      float tHit = tMin;
      Vector3 nL = enterN;

      if (enterN.LengthSq() < 1e-12f) {
         tHit = tMax;
         nL = exitN;
      }
      else if (tHit < ray.tMin) {
         tHit = tMax;
         nL = exitN;
      }

      if (tHit < ray.tMin || tHit > ray.tMax)
         return false;
      
      if (nL.LengthSq() < 1e-12f) {
         const Vector3 localP = oL + dL * tHit;

         const float dx = expandedHalf.x - std::abs(localP.x);
         const float dy = expandedHalf.y - std::abs(localP.y);
         const float dz = expandedHalf.z - std::abs(localP.z);

         if (dx <= dy && dx <= dz)
            nL = Vector3{ localP.x >= 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f };
         else if (dy <= dz)
            nL = Vector3{ 0.0f, localP.y >= 0.0f ? 1.0f : -1.0f, 0.0f };
         else
            nL = Vector3{ 0.0f, 0.0f, localP.z >= 0.0f ? 1.0f : -1.0f };
      }

      outHit.hit = true;
      outHit.t = tHit;

      // 주의:
      // 이 point는 "투사체 중심 위치" 기준의 충돌 시점 위치임.
      // 실제 표면 접촉점이 아님.
      outHit.point = ray.At(tHit);

      Vector3 nW = worldAxis_[0] * nL.x
                 + worldAxis_[1] * nL.y
                 + worldAxis_[2] * nL.z;

      outHit.normal = nW;
      outHit.collider = this;

      assert(SE::Math::NearlyZero(nW.LengthSq() - 1.0f, 1e-3f) && "OBB SphereCast normal must be normalized");

      return true;
   }

   SE::Math::Vector3 OBBCollider::ClosestPoint(const Vector3& point) const
   {
      const Vector3 d = point - worldCenter_;
      
      Vector3 q = worldCenter_;
      
      float dist = d.Dot(worldAxis_[0]);
      dist = SE::Math::Max(-worldHalf_.x, SE::Math::Min(dist , worldHalf_.x));
      q = q + (worldAxis_[0] * dist);
      
      dist = d.Dot(worldAxis_[1]);
      dist = SE::Math::Max(-worldHalf_.y, SE::Math::Min(dist , worldHalf_.y));
      q = q + (worldAxis_[1] * dist);
      
      dist = d.Dot(worldAxis_[2]);
      dist = SE::Math::Max(-worldHalf_.z, SE::Math::Min(dist , worldHalf_.z));
      q = q + (worldAxis_[2] * dist);
      
      return q;
   }

   void OBBCollider::RecalcWorldAABB()
   {
      Vector3 r;
      r.x = SE::Math::Abs(worldAxis_[0].x) * worldHalf_.x + SE::Math::Abs(worldAxis_[1].x) * worldHalf_.y + SE::Math::Abs(worldAxis_[2].x) * worldHalf_.z;
      r.y = SE::Math::Abs(worldAxis_[0].y) * worldHalf_.x + SE::Math::Abs(worldAxis_[1].y) * worldHalf_.y + SE::Math::Abs(worldAxis_[2].y) * worldHalf_.z;
      r.z = SE::Math::Abs(worldAxis_[0].z) * worldHalf_.x + SE::Math::Abs(worldAxis_[1].z) * worldHalf_.y + SE::Math::Abs(worldAxis_[2].z) * worldHalf_.z;
      
      const Vector3 mn = worldCenter_ - r;
      const Vector3 mx = worldCenter_ + r;
      
      worldAABB_.SetMinMax(mn, mx);
   }
}

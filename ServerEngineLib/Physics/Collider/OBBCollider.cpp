#include "pch.h"
#include "OBBCollider.h"

/*---------------
   OBBCollider
---------------*/

namespace SE::Physics
{
   static inline float AbsF(float v) { return (v < 0.0f) ? -v : v; }
   static inline float MinF(float a, float b) { return (a < b) ? a : b; }
   static inline float MaxF(float a, float b) { return (a > b) ? a : b; }
   
   static inline bool NearlyZero(float v, float eps = 1e-4f) { return AbsF(v) < eps; }
   
   //-----------------------------------------------------------------------------
   //-----------------------------------------------------------------------------
   
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
      
      half_.x = AbsF(halfExtent.x);
      half_.y = AbsF(halfExtent.y);
      half_.z = AbsF(halfExtent.z);
      
      axis_[0] = axisX.Normalized(Vector3{1, 0, 0});
      axis_[1] = axisY.Normalized(Vector3{0, 1, 0});
      axis_[2] = axisZ.Normalized(Vector3{0, 0, 1});
      
      // TODO: 디버그 일 때만 아래를 실행하도록 설정
      {
         assert(NearlyZero(axis_[0].Dot(axis_[1])) and "OBB axes must be orthogonal");
         assert(NearlyZero(axis_[0].Dot(axis_[2])) and "OBB axes must be orthogonal");
         assert(NearlyZero(axis_[1].Dot(axis_[2])) and "OBB axes must be orthogonal");
      }
      
      RecalcWorldAABB();
   }

   const AABBCollider& OBBCollider::GetWorldAABB() const
   {
      return worldAABB_;
   }

   Vector3 OBBCollider::ClosestPoint(const Vector3& point) const
   {
      const Vector3 d = point - center_;
      
      Vector3 q = center_;
      
      float dist = d.Dot(axis_[0]);
      dist = MaxF(-half_.x, MinF(dist , half_.x));
      q = q + (axis_[0] * dist);
      
      dist = d.Dot(axis_[1]);
      dist = MaxF(-half_.y, MinF(dist , half_.y));
      q = q + (axis_[1] * dist);
      
      dist = d.Dot(axis_[2]);
      dist = MaxF(-half_.z, MinF(dist , half_.z));
      q = q + (axis_[2] * dist);
      
      return q;
   }

   void OBBCollider::RecalcWorldAABB()
   {
      Vector3 r;
      r.x = AbsF(axis_[0].x) * half_.x + AbsF(axis_[1].x) * half_.y + AbsF(axis_[2].x) * half_.z;
      r.y = AbsF(axis_[0].y) * half_.x + AbsF(axis_[1].y) * half_.y + AbsF(axis_[2].y) * half_.z;
      r.z = AbsF(axis_[0].z) * half_.x + AbsF(axis_[1].z) * half_.y + AbsF(axis_[2].z) * half_.z;
      
      const Vector3 mn = center_ - r;
      const Vector3 mx = center_ + r;
      
      worldAABB_.SetMinMax(mn, mx);
   }
}

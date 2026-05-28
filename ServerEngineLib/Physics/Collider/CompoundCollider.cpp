#include "pch.h"
#include "CompoundCollider.h"
#include "CollisionResult.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"

/*--------------------
   CompoundCollider
--------------------*/

namespace SE::Physics
{
   ColliderType CompoundCollider::GetType() const
   {
      return ColliderType::Compound;
   }

   Collider* CompoundCollider::Clone() const
   {
      auto* c = new CompoundCollider();
      c->Reserve(colliders_.size());
      
      for (auto& child : colliders_)
         c->Add(std::unique_ptr<Collider>(child->Clone()));
      
      c->dirtyAABB_ = true;
      
      return c;
   }

   void CompoundCollider::UpdateWorld(const Math::Vector3& position, float yaw)
   {
      for (auto& child : colliders_) {
         if (child)
            child->UpdateWorld(position, yaw);
      }
      
      dirtyAABB_ = true;
   }

   bool CompoundCollider::Intersect(const Collider& other, CollisionResult& out) const
   {
      bool hit = false;
      CollisionResult bestResult;
      float bestPen = -1.0f;
      
      const AABBCollider& otherAabb = other.GetWorldAABB();

      for (const auto& child : colliders_) {
         if (not child) continue;
         
         CollisionResult tmpAabb;
         if (!child->GetWorldAABB().Intersect(otherAabb, tmpAabb))
            continue;
         
         CollisionResult tempResult;
         if (child->Intersect(other, tempResult)) {
            
            if (tempResult.penetration < 0.0f) continue; // 방어

            // 정책: penetration이 가장 큰 충돌 결과를 선택
            if (not hit or tempResult.penetration > bestPen) {
               hit = true;
               bestPen = tempResult.penetration;
               bestResult = tempResult;
            }
         }
      }
      
      if (not hit) return false;
      
      out = bestResult;
      return true;
   }

   void CompoundCollider::Reserve(size_t n)
   {
      colliders_.reserve(n);
   }

   void CompoundCollider::Clear()
   {
      colliders_.clear();
      dirtyAABB_ = true;
   }

   void CompoundCollider::Add(ColliderRef collider)
   {
      if (!collider) {
         // TODO: 디버그 일 때만 아래를 실행하도록 설정
         {
            assert(false && "CompoundCollider::Add - collider is nullptr");
         }
         return;
      }
      colliders_.push_back(std::move(collider));
      dirtyAABB_ = true;
   }

   bool CompoundCollider::ContainsPoint(const Math::Vector3& point) const
   {
      // Broadphase
      if (!GetWorldAABB().ContainsPoint(point))
         return false;

      for (const auto& child : colliders_)
      {
         if (!child)
            continue;

         // Child AABB broadphase
         if (!child->GetWorldAABB().ContainsPoint(point))
            continue;

         if (child->ContainsPoint(point))
            return true;
      }

      return false;
   }

   bool CompoundCollider::ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest,
      Math::Vector3& outNormal) const
   {
      if (colliders_.empty())
         return false;

      bool found = false;

      Vector3 bestClosest{};
      Vector3 bestNormal{};

      float bestDistSq = std::numeric_limits<float>::max();

      // 1. point가 Compound AABB와 너무 동떨어져 있으면 그래도 child 검사 가능
      //    ClosestPointOnSurface는 "가장 가까운 표면" 용도라 AABB contains로 early return 하면 안 됨.
      for (const auto& child : colliders_)
      {
         if (!child)
            continue;

         Vector3 childClosest{};
         Vector3 childNormal{};

         if (!child->ClosestPointOnSurface(point, childClosest, childNormal))
            continue;

         const float distSq = (point - childClosest).LengthSq();

         if (!found || distSq < bestDistSq)
         {
            found = true;
            bestDistSq = distSq;
            bestClosest = childClosest;
            bestNormal = childNormal;
         }
      }

      if (!found)
         return false;

      outClosest = bestClosest;
      outNormal = bestNormal;
      return true;
   }

   const AABBCollider& CompoundCollider::GetWorldAABB() const
   {
      if (dirtyAABB_)
         RecalcWorldAABB();
      
      return worldAABB_;
   }

   bool CompoundCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      // 충돌체가 없다면... 무조건 실패
      if (colliders_.empty())
         return false;
      
      // Broadphase - Compound AABB와 충돌 검사
      RaycastHit tempAABB;
      const AABBCollider& world = GetWorldAABB();
      if (not world.Raycast(ray, tempAABB))
         return false;
      
      bool hit = false;
      RaycastHit bestHit;
      float bestT = ray.tMax;
      
      for (const auto& child : colliders_) {
         
         if (not child) continue;
         
         // QUES: Broadphase - 자식 충돌체 AABB와 충돌 검사를 먼저 할까?
         RaycastHit tempChildAABB;
         if (not child->GetWorldAABB().Raycast(ray, tempChildAABB))
            continue;
         
         Ray localRay = ray;
         localRay.tMax = bestT;
         
         RaycastHit h;
         if (child->Raycast(localRay, h)) {
            if (not hit or h.t < bestT) {
               hit = true;
               bestT = h.t;
               bestHit = h;
            }
         }
      }
      
      if (not hit)
         return false;
      
      out = bestHit;
      // QUES: out.collider를 자식 Collider를 가리키게 하는 것이 유리하다
      // out.collider = this; <- nono
      
      return true;
   }

   bool CompoundCollider::SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius,
      RaycastHit& outHit) const
   {
      if (colliders_.empty())
         return false;
      
      bool hit = false;
      float bestT = std::numeric_limits<float>::max();
      RaycastHit bestHit;
      
      for (const auto& child : colliders_) {
         if (not child) continue;
         
         RaycastHit tempAABB;
         if (not child->GetWorldAABB().SphereCast(from, to, radius, tempAABB))
            continue;
         
         RaycastHit h;
         if (child->SphereCast(from, to, radius, h)) {
            if (not hit or h.t < bestT) {
               hit = true;
               bestT = h.t;
               bestHit = h;
            }
         }
      }
      
      if (not hit)
         return false;
      
      outHit = bestHit;
      return true;
   }

   void CompoundCollider::RecalcWorldAABB() const
   {
      dirtyAABB_ = false;
      
      if (colliders_.empty()) {
         worldAABB_.SetMinMax(Vector3(0, 0, 0), Vector3(0, 0, 0));
         return;
      }
      
      // Initialize with the first collider's AABB
      const AABBCollider& first = colliders_[0]->GetWorldAABB();
      Vector3 mn = first.GetMin();
      Vector3 mx = first.GetMax();
      
      // Expand to include all other colliders' AABBs
      for (size_t i = 1; i < colliders_.size(); ++i) {
         
         const AABBCollider& aabb = colliders_[i]->GetWorldAABB();
         
         const Vector3& cmn = aabb.GetMin();
         const Vector3& cmx = aabb.GetMax();
         
         mn.x =SE::Math::Min(mn.x, cmn.x);
         mn.y =SE::Math::Min(mn.y, cmn.y);
         mn.z =SE::Math::Min(mn.z, cmn.z);
         
         mx.x =SE::Math::Max(mx.x, cmx.x);
         mx.y =SE::Math::Max(mx.y, cmx.y);
         mx.z =SE::Math::Max(mx.z, cmx.z);
      }
      
      worldAABB_.SetMinMax(mn, mx);
   }
}

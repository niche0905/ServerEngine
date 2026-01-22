#include "pch.h"
#include "CompoundCollider.h"

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

   bool CompoundCollider::Intersect(const Collider& other, CollisionResult& out) const
   {
      // TODO: 나중에
      return false;
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

   const AABBCollider& CompoundCollider::GetWorldAABB() const
   {
      if (dirtyAABB_)
         RecalcWorldAABB();
      
      return worldAABB_;
   }

   bool CompoundCollider::Raycast(const Ray& ray, RaycastHit& out) const
   {
      return false;
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

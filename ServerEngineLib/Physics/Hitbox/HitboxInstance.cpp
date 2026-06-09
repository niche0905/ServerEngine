#include "pch.h"
#include "HitboxInstance.h"
#include "Physics/Collider/Collider.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"

/*------------------
   HitboxInstance
------------------*/

namespace SE::Physics::Hit
{
   namespace
   {
      void FillHitResultFromPart(const HitboxPart& part, uint16 partIndex, const RaycastHit& rayHit, HitResult& out)
      {
         out.hit = true;
         out.t = rayHit.t;
         out.point = rayHit.point;
         out.normal = rayHit.normal;
         out.group = part.group;
         out.damageMultiplier = part.damageMultiplier;
         out.partIndex = partIndex;
      }
   }

   void HitboxInstance::Bind(std::vector<HitboxPart> parts)
   {
      parts_ = std::move(parts);
      worldAABB_.SetMinMax(Vector3{}, Vector3{});
   }

   void HitboxInstance::Clear()
   {
      parts_.clear();
      worldAABB_.SetMinMax(Vector3{}, Vector3{});
   }

   void HitboxInstance::Update(const Vector3& rootPos, float yawDegrees)
   {
      if (parts_.empty()) {
         worldAABB_.SetMinMax(rootPos, rootPos);
         return;
      }

      bool hasAABB = false;
      Vector3 minPoint{};
      Vector3 maxPoint{};

      for (HitboxPart& part : parts_) {
         if (!part.collider)
            continue;

         part.collider->UpdateWorld(rootPos, yawDegrees);

         const AABBCollider& aabb = part.collider->GetWorldAABB();
         if (!hasAABB) {
            minPoint = aabb.GetMin();
            maxPoint = aabb.GetMax();
            hasAABB = true;
         }
         else {
            minPoint.x = Math::Min(minPoint.x, aabb.GetMin().x);
            minPoint.y = Math::Min(minPoint.y, aabb.GetMin().y);
            minPoint.z = Math::Min(minPoint.z, aabb.GetMin().z);
            maxPoint.x = Math::Max(maxPoint.x, aabb.GetMax().x);
            maxPoint.y = Math::Max(maxPoint.y, aabb.GetMax().y);
            maxPoint.z = Math::Max(maxPoint.z, aabb.GetMax().z);
         }
      }

      if (hasAABB) {
         worldAABB_.SetMinMax(minPoint, maxPoint);
      }
      else {
         worldAABB_.SetMinMax(rootPos, rootPos);
      }
   }

   const AABBCollider& HitboxInstance::GetWorldAABB() const
   {
      return worldAABB_;
   }

   bool HitboxInstance::Raycast(const Ray& ray, HitResult& out) const
   {
      if (parts_.empty())
         return false;

      RaycastHit aabbHit{};
      if (!worldAABB_.Raycast(ray, aabbHit))
         return false;

      bool hit = false;
      float bestT = ray.tMax;
      HitResult bestHit{};

      for (size_t i = 0; i < parts_.size(); ++i) {
         const HitboxPart& part = parts_[i];
         if (!part.collider)
            continue;

         RaycastHit partAabbHit{};
         if (!part.collider->GetWorldAABB().Raycast(ray, partAabbHit))
            continue;

         Ray partRay = ray;
         partRay.tMax = bestT;

         RaycastHit rayHit{};
         if (!part.collider->Raycast(partRay, rayHit))
            continue;

         if (!rayHit.hit || rayHit.t >= bestT)
            continue;

         FillHitResultFromPart(part, static_cast<uint16>(i), rayHit, bestHit);
         bestT = rayHit.t;
         hit = true;
      }

      if (!hit)
         return false;

      out = bestHit;
      return true;
   }

   bool HitboxInstance::SphereCast(const Vector3& from, const Vector3& to, float radius, HitResult& out) const
   {
      if (parts_.empty())
         return false;

      RaycastHit aabbHit{};
      if (!worldAABB_.SphereCast(from, to, radius, aabbHit))
         return false;

      bool hit = false;
      float bestT = std::numeric_limits<float>::max();
      HitResult bestHit{};

      for (size_t i = 0; i < parts_.size(); ++i) {
         const HitboxPart& part = parts_[i];
         if (!part.collider)
            continue;

         RaycastHit partAabbHit{};
         if (!part.collider->GetWorldAABB().SphereCast(from, to, radius, partAabbHit))
            continue;

         RaycastHit sweepHit{};
         if (!part.collider->SphereCast(from, to, radius, sweepHit))
            continue;

         if (!sweepHit.hit || sweepHit.t >= bestT)
            continue;

         FillHitResultFromPart(part, static_cast<uint16>(i), sweepHit, bestHit);
         bestT = sweepHit.t;
         hit = true;
      }

      if (!hit)
         return false;

      out = bestHit;
      return true;
   }

   bool HitboxInstance::Intersect(const Collider& other, HitResult* out) const
   {
      if (parts_.empty())
         return false;

      CollisionResult aabbResult{};
      if (!worldAABB_.Intersect(other.GetWorldAABB(), aabbResult))
         return false;

      bool hit = false;
      float bestPenetration = -1.0f;
      HitResult bestHit{};

      for (size_t i = 0; i < parts_.size(); ++i) {
         const HitboxPart& part = parts_[i];
         if (!part.collider)
            continue;

         CollisionResult partAabbResult{};
         if (!part.collider->GetWorldAABB().Intersect(other.GetWorldAABB(), partAabbResult))
            continue;

         CollisionResult collisionResult{};
         if (!part.collider->Intersect(other, collisionResult) || !collisionResult.hit)
            continue;

         if (hit && collisionResult.penetration <= bestPenetration)
            continue;

         bestPenetration = collisionResult.penetration;
         bestHit.hit = true;
         bestHit.point = collisionResult.point;
         bestHit.normal = collisionResult.normal;
         bestHit.group = part.group;
         bestHit.damageMultiplier = part.damageMultiplier;
         bestHit.partIndex = static_cast<uint16>(i);
         hit = true;
      }

      if (!hit)
         return false;

      if (out)
         *out = bestHit;

      return true;
   }
}

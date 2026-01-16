#include "pch.h"
#include "AABBCollider.h"

/*----------------
   AABBCollider
----------------*/

namespace SE::Physics
{
   static inline float MinF(float a, float b) { return a < b ? a : b; }
   static inline float MaxF(float a, float b) { return a > b ? a : b; }
   
   AABBCollider::AABBCollider(const Vector3& minPoint, const Vector3& maxPoint)
   {
      SetMinMax(minPoint, maxPoint);
   }

   ColliderType AABBCollider::GetType() const
   {
      return ColliderType::AABB;
   }

   Collider* AABBCollider::Clone() const
   {
      return new AABBCollider(*this);
   }

   bool AABBCollider::Intersect(const Collider& other, CollisionResult& out) const
   {
      switch (other.GetType())
      {
      case ColliderType::AABB:
         {
            const auto& b = static_cast<const AABBCollider&>(other);
            const bool hit = Overlaps(b);
            
            // TODO: 정확한 CollisionResult가 필요하면 여기에서 채워넣어야 한다
            
            out.hit = hit;
            return hit;
         }
         
         // TODO: 충돌체 들 구현 되면 여기에 추가
      default:
         out.hit = false;
         return false;
      }
   }

   void AABBCollider::SetMinMax(const Vector3& minPoint, const Vector3& maxPoint)
   {
      min_.x = MinF(minPoint.x, maxPoint.x);
      min_.y = MinF(minPoint.y, maxPoint.y);
      min_.z = MinF(minPoint.z, maxPoint.z);
      
      max_.x = MaxF(minPoint.x, maxPoint.x);
      max_.y = MaxF(minPoint.y, maxPoint.y);
      max_.z = MaxF(minPoint.z, maxPoint.z);
      
      RecaleCache();
   }

   bool AABBCollider::Contains(const Vector3& point) const
   {
      return (point.x >= min_.x && point.x <= max_.x) and 
         (point.y >= min_.y && point.y <= max_.y) and
         (point.z >= min_.z && point.z <= max_.z);
   }

   bool AABBCollider::Overlaps(const AABBCollider& other) const
   {
      if (max_.x < other.min_.x or min_.x > other.max_.x) return false;
      if (max_.y < other.min_.y or min_.y > other.max_.y) return false;
      if (max_.z < other.min_.z or min_.z > other.max_.z) return false;
      
      return true;
   }

   void AABBCollider::Expand(float margin)
   {
      min_.x -= margin; min_.y -= margin; min_.z -= margin;
      max_.x += margin; max_.y += margin; max_.z += margin;
      
      RecaleCache();
   }

   AABBCollider AABBCollider::Union(const AABBCollider& a, const AABBCollider& b)
   {
      AABBCollider result;
      Vector3 mn{
         MinF(a.min_.x, b.min_.x),
         MinF(a.min_.y, b.min_.y),
         MinF(a.min_.z, b.min_.z)
      };
      Vector3 mx{
         MaxF(a.max_.x, b.max_.x),
         MaxF(a.max_.y, b.max_.y),
         MaxF(a.max_.z, b.max_.z)
      };
      result.SetMinMax(mn, mx);
      
      return result;
   }

   void AABBCollider::RecaleCache()
   {
      center_ = (min_ + max_) * 0.5f;
      extent_ = (max_ - min_) * 0.5f;
   }
}

#include "pch.h"
#include "IntersectTable.h"
#include "IntersectFns.h"

/*---------------
   Narrowphase
---------------*/

namespace SE::Physics::Narrowphase
{
   constexpr int kTypeCount = static_cast<int>(ColliderType::Compound) + 1;
   
   static IntersectFn gTable[kTypeCount][kTypeCount];
   
   static bool gInited = false;
   
   // 구현이 없는 충돌 판정 함수
   static bool NotSupported(const Collider&, const Collider&, CollisionResult&)
   {
      return false;
   }
   
   void InitIntersectTable()
   {
      // 모든 조합을 NotSupported로 초기화
      for (int i = 0; i < kTypeCount; ++i) {
         for (int j = 0; j < kTypeCount; ++j) {
            gTable[i][j] = &NotSupported;
         }
      }
      
      // 지원하는 충돌 판정 함수 등록
      // 예: gTable[static_cast<int>(ColliderType::AABB)][static_cast<int>(ColliderType::OBB)] = AABBvsOBB;
      // 실제 구현된 함수로 교체 필요
      
      // TODO: 충돌 판정 함수 구현 후 등록
      gTable[static_cast<int>(ColliderType::AABB)][static_cast<int>(ColliderType::AABB)] = &Intersect_AABB_AABB;
      gTable[static_cast<int>(ColliderType::AABB)][static_cast<int>(ColliderType::OBB)] = &Intersect_AABB_OBB;
      gTable[static_cast<int>(ColliderType::OBB)][static_cast<int>(ColliderType::AABB)] = &Intersect_OBB_AABB;
      gTable[static_cast<int>(ColliderType::AABB)][static_cast<int>(ColliderType::Sphere)] = &Intersect_AABB_Sphere;
      gTable[static_cast<int>(ColliderType::Sphere)][static_cast<int>(ColliderType::AABB)] = &Intersect_Sphere_AABB;
      gTable[static_cast<int>(ColliderType::AABB)][static_cast<int>(ColliderType::Capsule)] = &Intersect_AABB_Capsule;
      gTable[static_cast<int>(ColliderType::Capsule)][static_cast<int>(ColliderType::AABB)] = &Intersect_Capsule_AABB;
      
      gInited = true;
   }
   
   IntersectFn GetIntersectFn(ColliderType a, ColliderType b)
   {
      assert(gInited && "InitIntersectTable must be called before GetIntersectFn()");
      return gTable[static_cast<int>(a)][static_cast<int>(b)];
   }
   
}
#pragma once
#include "Physics/Collider/Collider.h"

/*----------------
   IntersectFns
----------------*/
//
// IntersectFns 헤더는 실제 구현될 충돌 판정 함수들의 선언을 포함합니다
//

namespace SE::Physics::Narrowphase
{
   
   bool Intersect_AABB_AABB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_AABB_OBB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_OBB_AABB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_AABB_Sphere(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Sphere_AABB(const Collider& a, const Collider& b, CollisionResult& out);
   
}

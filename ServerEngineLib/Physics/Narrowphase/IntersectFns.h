#pragma once
#include "Physics/Collider/Collider.h"

/*----------------
   IntersectFns
----------------*/
//
// IntersectFns 헤더는 실제 구현될 충돌 판정 함수들의 선언을 포함합니다
//

namespace SE::Physics::NarrowPhase
{
   
   bool Intersect_AABB_AABB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_AABB_OBB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_OBB_AABB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_AABB_Sphere(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Sphere_AABB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_AABB_Capsule(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Capsule_AABB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_AABB_CharacterCapsule(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_CharacterCapsule_AABB(const Collider& a, const Collider& b, CollisionResult& out);
   
   bool Intersect_OBB_OBB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_OBB_Sphere(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Sphere_OBB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_OBB_Capsule(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Capsule_OBB(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_OBB_CharacterCapsule(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_CharacterCapsule_OBB(const Collider& a, const Collider& b, CollisionResult& out);
   
   bool Intersect_Sphere_Sphere(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Sphere_Capsule(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Capsule_Sphere(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Sphere_CharacterCapsule(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_CharacterCapsule_Sphere(const Collider& a, const Collider& b, CollisionResult& out);
   
   bool Intersect_Capsule_Capsule(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_Capsule_CharacterCapsule(const Collider& a, const Collider& b, CollisionResult& out);
   bool Intersect_CharacterCapsule_Capsule(const Collider& a, const Collider& b, CollisionResult& out);
   
   bool Intersect_CharacterCapsule_CharacterCapsule(const Collider& a, const Collider& b, CollisionResult& out);
   
   
}

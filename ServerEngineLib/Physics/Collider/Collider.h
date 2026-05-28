#pragma once

/*------------
   Collider
------------*/
//
// Collider는 서버 컨텐츠에서 사용하기 위해 만든 충돌체 클래스 입니다
//


namespace SE::Physics
{
    struct CollisionResult;
    class AABBCollider;
    struct Ray;
    struct RaycastHit;
    
    enum class ColliderType : uint8
    {
        AABB,
        OBB,
        Sphere,
        Capsule,
        CharacterCapsule,
        
        Compound,
    };
    
    class Collider
    {
    public:
        virtual ~Collider() = default;
        
        virtual ColliderType GetType() const = 0;
        virtual Collider* Clone() const = 0;
        
        virtual void UpdateWorld(const Math::Vector3& position, float yaw) = 0;
        
        virtual bool Intersect(const Collider& other, CollisionResult& out) const;
        virtual bool ContainsPoint(const Math::Vector3& point) const = 0;
        virtual bool ClosestPointOnSurface(const Math::Vector3& point, Math::Vector3& outClosest, Math::Vector3& outNormal) const = 0;
        
        virtual const AABBCollider& GetWorldAABB() const = 0;
        
        virtual bool Raycast(const Ray& ray, RaycastHit& out) const = 0;
        virtual bool SphereCast(const Math::Vector3& from, const Math::Vector3& to, float radius, RaycastHit& outHit) const = 0;
    };
    
}
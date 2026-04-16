#pragma once
#include "Actor.h"

class ObjectManager;

/*-------------------
   ProjectileActor
-------------------*/
//
// ProjectileActor는 발사체(총알, 미사일 등)를 나타내는 클래스입니다.
// 발사체는 소유자, 데미지, 속도, 수명 등의 속성을 가지며,
// 충돌 시 특정 동작을 수행할 것을 예상하여 설계되었습니다.
//

class ProjectileActor : public Actor
{
public:
    ProjectileActor() = default;
    virtual ~ProjectileActor() = default;
    
    ProjectileActor(const ProjectileActor&) = delete;
    ProjectileActor& operator=(const ProjectileActor&) = delete;
    
public:
   virtual se::common::ObjectType GetObjectType() const override { return se::common::OBJ_PROJECTILE; }
    
public:
    ObjectId GetOwner() const { return ownerId_; }
    void SetOwner(ObjectId ownerId) { ownerId_ = ownerId; }
    
    int32 GetDamage() const { return damage_; }
    void SetDamage(int32 damage) { damage_ = damage; }
    
    const Vector3& GetVelocity() const { return velocity_; }
    void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
    
public:
    void Init(ObjectId ownerId, const Vector3& startPos, const Vector3& velocity, int32 damage, uint32 lifetimeMs, float radius);
    
protected:
    virtual void OnSpawn() override;
    virtual void Tick(float dt) override;
    
public:
    void HandleLifetimeExpired();
    
protected:
    virtual void UpdateMovement(float dt);
    virtual void OnHit(ObjectManager& om, ObjectId hitObjectId);
    virtual void OnLifetimeExpired(ObjectManager& om);
    virtual void OnExplode(ObjectManager& om);
    
private:
    ObjectId ownerId_;
    int32 damage_{ 0 };
    
    TimerId lifetimeTimer_{};
    Vector3 velocity_{};
    
};

#include "pch.h"
#include "WorldItemActor.h"

/*-------------------
   WorldItemActor
-------------------*/

void WorldItemActor::InitDrop(const ItemStack& stack, const Vector3& pos, const Vector3& initVel)
{
    stack_ = stack;
    SetPosition(pos);
    velocity_ = initVel;
    
    sleeping_ = false;
    sleepTimeAcc_ = 0.0f;
}

void WorldItemActor::OnSpawn()
{
    Actor::OnSpawn();
}

void WorldItemActor::Tick(float dt)
{
    Actor::Tick(dt);
    
    if (sleeping_) 
        return;
    
    IntegratePhysics(dt);
    
    TrySleep(dt);
}

void WorldItemActor::IntegratePhysics(float dt)
{
    Vector3 pos = GetPosition();
    
    velocity_.y += gravity_ * dt;
    
    const float damp = (linearDamping_ > 0.0f) ? (1.0f - linearDamping_ * dt) : 1.0f;
    if (damp > 0.0f) {
        velocity_.x *= damp;
        velocity_.y *= damp;
        velocity_.z *= damp;
    }
    
    pos.x += velocity_.x * dt;
    pos.y += velocity_.y * dt;
    pos.z += velocity_.z * dt;
    
    // TODO: QueryGroundHeight 사용
    const float groundY = 0.0f;
    if (pos.y <= groundY) {
        pos.y = groundY;
        
        if (velocity_.y < 0.0f)
            velocity_.y = -velocity_.y * restitution_;
            
        const float friction = (groundFriction_ > 0.0f) ? (1.0f - groundFriction_ * dt) : 1.0f;
        if (friction > 0.0f) {
            velocity_.x *= friction;
            velocity_.z *= friction;
        }
        
        // 매우 작은 y 튕김일 때 잔진동 방지용
        if (velocity_.y * velocity_.y < 0.01f) {
            velocity_.y = 0.0f;
        }
    }
    
    SetPosition(pos);
}

float WorldItemActor::QueryGroundHeight(ObjectManager& om, const Vector3& pos) const
{
    (void)om;
    (void)pos;
    
    // TODO: Spatial/Physics 시스템에서 지면 높이 쿼리
    
    return 0.0f;
}

void WorldItemActor::TrySleep(float dt)
{
    const float velSq = velocity_.LengthSq();
    
    if (velSq <= sleepVelSq_) {
        sleepTimeAcc_ += dt;
        
        if (sleepTimeAcc_ >= sleepTimeReq_) {
            sleeping_ = true;
            velocity_ = Vector3{};
        }
    } else {
        sleepTimeAcc_ = 0.0f;
    }
    
}

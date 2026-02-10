#include "pch.h"
#include "ProjectileActor.h"

/*-------------------
   ProjectileActor
-------------------*/

void ProjectileActor::Init(ObjectId ownerId, const Vector3& startPos, const Vector3& velocity, int32 damage,
    uint32 lifetimeMs)
{
    ownerId_ = ownerId;
    damage_ = damage;
    velocity_ = velocity;
    lifetimeMs_ = lifetimeMs;
    
    SetPosition(startPos);
}

void ProjectileActor::OnSpawn()
{
    Actor::OnSpawn();
    
    // TODO: Register Lifetime 등록하기
    // RegisterLifetimeMs(nowMs, delay)
}

void ProjectileActor::Tick(float dt)
{
    Actor::Tick(dt);
    
    Vector3 pos = GetPosition();
    pos.x += velocity_.x * dt;
    pos.y += velocity_.y * dt;
    pos.z += velocity_.z * dt;
    SetPosition(pos);
    
    // TODO: 충돌 검사 및 OnHit 호출
    // Spatial/Collider 시스템 필요
}


void ProjectileActor::OnHit(ObjectManager& om, ObjectId hitObjectId)
{
    (void)om;
    (void)hitObjectId;
 
    // TODO: 아래 동작 구현
    // 1) 데미지 계산 식에 맞게 적용 (범위, 방어구 등)
    // 2) 자신은 제거 필요 시 EffectArea 생성 등 추가 동작
    
    // 기본적으로 충돌 시 파괴
    // __RequestDestroy();
}

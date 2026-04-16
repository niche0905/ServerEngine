#include "pch.h"
#include "ProjectileActor.h"

#include "Content/Object/ObjectManager.h"
#include "Physics/Collider/SphereCollider.h"
#include "Service/Room/Room.h"
#include "Shard/GameShard.h"

/*-------------------
   ProjectileActor
-------------------*/

void ProjectileActor::Init(ObjectId ownerId, const Vector3& startPos, const Vector3& velocity, int32 damage,
    uint32 lifetimeMs, float radius)
{
    ownerId_ = ownerId;
    damage_ = damage;
    velocity_ = velocity;
    
    auto room = GetRoom();
    if (!room)
        return;   // Room이 없는 경우, 유효하지 않은 상태
    
    auto& om = room->GetObjectManager();
    ObjectId objectId = GetId();
    RoomId roomId = room->GetRoomId();
    GameShard* ownerShard = room->GetOwnerShard();
    if (ownerShard == nullptr)
        return;
    
    // lifetime 타이머 설정 (발사체가 일정 시간이 지나면 자동으로 제거되도록)
    lifetimeTimer_ = room->ScheduleAfter(Milliseconds(lifetimeMs), [ownerShard, roomId, objectId]()
        {
            auto room = ownerShard->FindRoom(roomId);
            if (!room)
                return;   // Room이 없는 경우, 유효하지 않은 상태
            
            if (ProjectileActor* proj = room->GetObjectManager().FindAs<ProjectileActor>(objectId)) {
                proj->HandleLifetimeExpired();
            }
        });
    
    // 충돌체 설정
    {
        auto collider = std::make_unique<ColliderComponent>();
        auto sphereCollider = std::make_unique<SE::Physics::SphereCollider>(SE::Math::Vector3{0.0f, 0.0f, 0.0f}, radius);
        collider->Init(GetId(), this, ColliderRole::Hitbox, std::move(sphereCollider));
        
        colliders_.push_back(std::move(collider));
    }
    
    SetPosition(startPos);
}

void ProjectileActor::OnSpawn()
{
    Actor::OnSpawn();
}

void ProjectileActor::Tick(float dt)
{
    Actor::Tick(dt);
    
    UpdateMovement(dt);
    
    // TODO: 충돌 체크 및 처리 (예: Raycast 또는 Collider 간의 충돌 검사)
    //       충돌 사실 알리기 (OnHit 호출)
}

void ProjectileActor::HandleLifetimeExpired()
{
    if (not IsActive())
        return;
    
    auto room = GetRoom();
    if (!room)
        return;   // Room이 없는 경우, 유효하지 않은 상태
    
    OnLifetimeExpired(room->GetObjectManager());
}

void ProjectileActor::UpdateMovement(float dt)
{
    Vector3 pos = GetPosition();
    pos.x += velocity_.x * dt;
    pos.y += velocity_.y * dt;
    pos.z += velocity_.z * dt;
    SetPosition(pos);
}

void ProjectileActor::OnHit(ObjectManager& om, ObjectId hitObjectId)
{
    (void)om;
    (void)hitObjectId;
}

void ProjectileActor::OnLifetimeExpired(ObjectManager& om)
{
    OnExplode(om);
}

void ProjectileActor::OnExplode(ObjectManager& om)
{
    // TODO: 아래 동작 구현
    // 1) 데미지 계산 식에 맞게 적용 (범위, 방어구 등)
    // 2) 자신은 제거 필요 시 EffectArea 생성 등 추가 동작
    
    // 기본적으로 폭발 시 파괴
    if (lifetimeTimer_ != TimerId{}) {
        // Room에서 예약된 타이머 취소
        if (auto room = GetRoom()) {
            room->CancelScheduled(lifetimeTimer_);
        }
        lifetimeTimer_ = TimerId{};
    }
    om.RequestDestroy(GetId());
}

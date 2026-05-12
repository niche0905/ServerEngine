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
    uint32 lifetimeMs, float projectileRadius, float explosionRadius, bool distanceDamageEnabled)
{
    ownerId_ = ownerId;
    damage_ = damage;
    velocity_ = velocity;
    radius_ = explosionRadius;
    distanceDamageEnabled_ = distanceDamageEnabled;
    
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
        auto sphereCollider = std::make_unique<SE::Physics::SphereCollider>(SE::Math::Vector3{0.0f, 0.0f, 0.0f}, projectileRadius);
        collider->Init(this, ColliderRole::Hitbox, std::move(sphereCollider));
        
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
    const Vector3 oldPos = GetPosition();
    const Vector3 delta = velocity_ * dt;
    const Vector3 newPos = oldPos + delta;
    
    if (SE::Math::NearlyEqual(oldPos.x, newPos.x)
        and SE::Math::NearlyEqual(oldPos.y, newPos.y)
        and SE::Math::NearlyEqual(oldPos.z, newPos.z)) {
        
        return;
    }
    
    SetPosition(newPos);
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
    // 기본적으로 폭발 시 파괴
    if (lifetimeTimer_ != TimerId{}) {
        // Room에서 예약된 타이머 취소
        if (auto room = GetRoom()) {
            room->GetRoomGameSystem().GetCombatSystem().ProjectileExplosion(GetId(), GetPosition(), GetOwner(), GetDamage(), radius_, distanceDamageEnabled_);
            room->CancelScheduled(lifetimeTimer_);
        }
        else {
            consoleLogger->Log(Color::Yellow, L"[ProjectileActor] Warning: Room not found when trying to cancel lifetime timer.");
        }
        lifetimeTimer_ = TimerId{};
    }
    om.RequestDestroy(GetId());
}

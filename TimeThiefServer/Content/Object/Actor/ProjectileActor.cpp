#include "pch.h"
#include "ProjectileActor.h"

#include "Content/Object/ObjectManager.h"
#include "Physics/Collider/SphereCollider.h"
#include "Service/Room/Room.h"
#include "Service/Room/Systems/Combat/ProjectileSweepQuery.h"
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
    projectileRadius_ = projectileRadius;
    explosionRadius_ = explosionRadius;
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
    if (lifetimeMs > 0) {
        lifetimeTimer_ = room->ScheduleAfter(Milliseconds(lifetimeMs), [ownerShard, roomId, objectId]()
            {
                auto room = ownerShard->FindRoom(roomId);
                if (!room)
                    return;   // Room이 없는 경우, 유효하지 않은 상태
            
                if (ProjectileActor* proj = room->GetObjectManager().FindAs<ProjectileActor>(objectId)) {
                    proj->HandleLifetimeExpired();
                }
            });
    }
    
    // 충돌체 설정
    {
        auto collider = std::make_unique<ColliderComponent>();
        auto sphereCollider = std::make_unique<SE::Physics::SphereCollider>(SE::Math::Vector3{0.0f, 0.0f, 0.0f}, projectileRadius_);
        collider->Init(this, ColliderRole::Hit, std::move(sphereCollider));
        
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
    
    prevPos_ = GetPosition();
    
    UpdateMovement(dt);
    
    const Vector3 currPos = GetPosition();
    
    CheckHit(prevPos_, currPos);
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

void ProjectileActor::CheckHit(const SE::Math::Vector3& from, const SE::Math::Vector3& to)
{
    SE::Physics::Hit::HitResult hit{};
    
    if (auto room = GetRoom()) {
        ProjectileSweepQuery query{
            .projectileId = GetId(),
            .ownerId = GetOwner(),
            .from = from,
            .to = to,
            .radius = projectileRadius_,
            .hitMap = true,
            .hitBlockActor = true,
            .hitHurtBox = true
        };
        
        if (room->GetRoomGameSystem().GetCombatSystem().SweepProjectile(query, hit)) {
            if (hit.hit) {
                OnHit(room->GetObjectManager(), hit.actor ? hit.actor->GetId() : ObjectId{}, hit);
            }
        }
    }
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

void ProjectileActor::OnHit(ObjectManager& om, ObjectId hitObjectId, const SE::Physics::Hit::HitResult& hit)
{
    (void)om;
    (void)hitObjectId;
    
    constexpr float SurfaceOffset = 2.0f;

    if (hit.hit) {
        const SE::Math::Vector3 moveDir = velocity_; // 방향만 쓰니까 normalize 안 해도 됨

        SE::Math::Vector3 explodePos = ComputeExplosionPosition(hit, moveDir);

        SetPosition(explodePos);
    }
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
            room->NotifyExplosion(GetId(), GetOwner(), GetPosition(), explosionRadius_);
            room->GetRoomGameSystem().GetCombatSystem().ProjectileExplosion(GetId(), GetPosition(), GetOwner(), GetDamage(), explosionRadius_, distanceDamageEnabled_);
            room->CancelScheduled(lifetimeTimer_);
            room->HandleDespawn(GetId());
        }
        else {
            consoleLogger->Log(Color::Yellow, L"[ProjectileActor] Warning: Room not found when trying to cancel lifetime timer.");
        }
        lifetimeTimer_ = TimerId{};
    }
}

SE::Math::Vector3 ProjectileActor::ComputeExplosionPosition(const SE::Physics::Hit::HitResult& hit,
    const SE::Math::Vector3& moveDir) const
{
    constexpr float BackOffset = 2.0f;
    constexpr float NormalOffset = 1.0f;

    SE::Math::Vector3 pos = hit.point;

    // 우선 투사체 진행 방향 반대로 살짝 빼기
    if (moveDir.LengthSq() > 0.0001f) {
        pos = pos - moveDir.Normalized() * BackOffset;
    }

    // normal이 신뢰 가능하면 표면 밖으로 보정
    if (hit.normal.LengthSq() > 0.0001f) {
        pos = pos + hit.normal.Normalized() * NormalOffset;
    }

    return pos;
}

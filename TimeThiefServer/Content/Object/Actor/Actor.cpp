#include "pch.h"
#include "Actor.h"
#include "Service/Room/Systems/Replication/ReplicationSystem.h"
#include "Content/Gameplay/Spawn/SpawnService.h"

/*-----------------
   Local Helper
-----------------*/

namespace 
{
    // TODO: 구현 필요
    
    ObjectManager* TryGetObjectManager(RoomId /*roomId*/)
    {
        return nullptr;
    }
    
    SpawnService* TryGetSpawnService(RoomId /*roomId*/)
    {
        return nullptr;
    }
    
    ReplicationSystem* TryGetReplication(RoomId /*roomId*/)
    {
        return nullptr;
    }
}

/*---------
   Actor
---------*/

void Actor::SetPosition(const Vector3& position)
{
    position_ = position;
    SyncColliders();
    MarkReplicationDirty(ReplicationDirty::Transform);
}

void Actor::SetYaw(float yaw)
{
    yaw_ = yaw;
    SyncColliders();
    MarkReplicationDirty(ReplicationDirty::Transform);
}

void Actor::SetTransform(const Vector3& position, float yaw)
{
    position_ = position;
    yaw_ = yaw;
    SyncColliders();
    MarkReplicationDirty(ReplicationDirty::Transform);
}

void Actor::ForEachCollider(const std::function<void(ColliderComponent*)>& fn) const
{
    for (const auto& collider : colliders_) {
        fn(collider.get());
    }
}

void Actor::SyncColliders()
{
    for (auto& collider : colliders_) {
        if (collider) {
            collider->UpdateWorldCollider();
        }
    }
}

void Actor::RegisterLifetimeMs(uint64 nowMs, uint32 lifetimeMs)
{
    if (lifetimeMs == 0) return;
    
    if (auto* ss = TryGetSpawnService(GetRoomId())) {
        ss->RegisterLifetimeMs(GetId(), nowMs, lifetimeMs);
    }
}

#include "pch.h"
#include "Actor.h"
#include "Service/Room/Room.h"

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

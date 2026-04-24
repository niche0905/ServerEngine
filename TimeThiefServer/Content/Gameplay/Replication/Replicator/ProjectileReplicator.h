#pragma once
#include "Content/Gameplay/Replication/IObjectReplicator.h"

class ProjectileActor;

class ProjectileReplicator : public IObjectReplicator
{
public:
    virtual ReplicateResult FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const override;
    
private:
    ReplicateResult FlushProjectilePeriodic(ProjectileActor& projectile, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const;
    
};

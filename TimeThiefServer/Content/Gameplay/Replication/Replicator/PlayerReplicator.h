#pragma once
#include "Content/Gameplay/Replication/IObjectReplicator.h"

class PlayerPawn;

class PlayerReplicator : public IObjectReplicator
{
public:
    virtual ReplicateResult FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const;
    
private:
    virtual ReplicateResult FlushPlayerPeriodic(PlayerPawn& player, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const;
    
};

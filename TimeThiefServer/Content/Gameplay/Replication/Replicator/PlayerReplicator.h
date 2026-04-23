#pragma once
#include "Content/Gameplay/Replication/IObjectReplicator.h"

class PlayerPawn;

class PlayerReplicator : public IObjectReplicator
{
public:
    virtual bool FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const;
    
private:
    virtual bool FlushPlayerPeriodic(PlayerPawn& player, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const;
    
};

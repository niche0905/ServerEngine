#pragma once
#include "Content/Gameplay/Replication/IObjectReplicator.h"

class MonsterPawn;

class MonsterReplicator : public IObjectReplicator
{
public:
    virtual ReplicateResult FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const override;
    
private:
    ReplicateResult FlushMonsterPeriodic(MonsterPawn& monster, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const;
    
};

#include "pch.h"
#include "MonsterReplicator.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Service/Room/Room.h"


ReplicateResult MonsterReplicator::FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame,
    uint64 nowMs, Room& room) const
{
    auto* monster = dynamic_cast<MonsterPawn*>(obj);
    if (!monster)
        return ReplicateResult{};
    
    return FlushMonsterPeriodic(*monster, flags, frame, nowMs, room);
}

ReplicateResult MonsterReplicator::FlushMonsterPeriodic(MonsterPawn& monster, ReplicationDirty flags,
    const RepFrame& frame, uint64 nowMs, Room& room) const
{
    ReplicateResult result;
    
    // TODO: Monster Pawn과 BT 구조가 이루어지면 Replicate 대상을 다루도록
    
    return result;
}

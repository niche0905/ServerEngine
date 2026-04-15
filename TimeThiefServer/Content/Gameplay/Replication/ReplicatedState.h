#pragma once
#include "ReplicationTypes.h"


class ReplicatedState
{
public:
    void MarkDirty(ReplicationDirty flag) { dirtyFlags_ |= flag; }
    void ClearDirty() { dirtyFlags_ = ReplicationDirty::None; }
    
    bool IsDirty() const { return dirtyFlags_ != ReplicationDirty::None; }
    bool Has(ReplicationDirty flag) const { return HasDirty(dirtyFlags_, flag); }
    
    ReplicationDirty GetFlags() const { return dirtyFlags_; }
    
public:
    uint64 replicationVersion = 0;
    uint64 lastReplicatedTick = 0;
    
private:
    ReplicationDirty dirtyFlags_ = ReplicationDirty::None;
    
};

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
    
    TickSeq lastReplicatedTick = 0;     // 마지막으로 복제된 틱 시퀀스
    uint64 lastReplicatedTimeMs = 0;    // 마지막으로 복제된 절대/단조 ms 시각 (밀리초)
    
private:
    ReplicationDirty dirtyFlags_ = ReplicationDirty::None;
    
};

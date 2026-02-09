#pragma once
#include "ReplicationMask.h"
#include "ReplicationTypes.h"

struct ReplicatedState
{
    RepObjectId object{};
    RepMask dirtyMask{0};
    
    TickSeq lastFlushSeq{0};    // 마지막으로 flush한 Tick
    
    bool IsDirty() const { return dirtyMask != 0; }
    
    void MarkDirty(RepField f) { RepMaskSet(dirtyMask, f); }
    void ClearDirty() { dirtyMask = 0; }  // 전체 초기화
};

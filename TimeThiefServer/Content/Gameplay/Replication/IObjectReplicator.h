#pragma once
#include "ReplicationEvent.h"
#include "ReplicationTypes.h"

class Room;
class BaseObject;

class IObjectReplicator
{
public:
    virtual ~IObjectReplicator() = default;
    
    // virtual bool FlushImmediate(const RepEvent& ev, const RepFrame& frame, Room& room) const = 0;
    // ㄴ 이건 없는 게 맞을 듯? (Event는 ReplicationSystem의 FlushImmediate에서 RepEvent를 보고 처리하면 될 것이라 생각)
    virtual ReplicateResult FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const = 0;
    
};

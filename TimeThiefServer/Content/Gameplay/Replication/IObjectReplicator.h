#pragma once
#include "ReplicationEvent.h"
#include "ReplicationTypes.h"

class Room;
class BaseObject;

class IObjectReplicator
{
public:
    virtual ~IObjectReplicator() = default;
    
    virtual bool FlushImmediate(const RepEvent& ev, const RepFrame& frame, Room& room) const = 0;
    virtual bool FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs, Room& room) const = 0;
    
};

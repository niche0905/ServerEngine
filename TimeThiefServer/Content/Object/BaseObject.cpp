#include "pch.h"
#include "BaseObject.h"
#include "Service/Room/Room.h"

/*--------------
   BaseObject
--------------*/

void BaseObject::MarkReplicationDirty(ReplicationDirty dirtyFlag)
{
    if (not HasFlag(flags_, ObjectFlags::Replicable))
        return;
    
    replicated_.MarkDirty(dirtyFlag);
        
    if (auto room = GetRoom()) {
        room->GetRoomGameSystem().GetReplicationSystem().MarkDirty(GetId());
    }
}
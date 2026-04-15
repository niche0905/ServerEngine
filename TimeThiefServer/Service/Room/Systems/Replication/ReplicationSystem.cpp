#include "pch.h"
#include "ReplicationSystem.h"

/*---------------------
   ReplicationSystem
---------------------*/

bool ReplicationSystem::Init(Room* ownerRoom)
{
   if (!ownerRoom)
      return false;   // 유효하지 않은 ownerRoom
   
   ownerRoom_ = ownerRoom;
   
   return true;
}

void ReplicationSystem::NotifySpawn(BaseObject* object)
{
   
}

void ReplicationSystem::NotifyDespawn(ObjectId objectId)
{
}

void ReplicationSystem::PushEvent(const RepEvent& event)
{
}

void ReplicationSystem::MarkDirty(ObjectId objectId)
{
}

void ReplicationSystem::FlushImmediate(const RepFrame& frame)
{
}

void ReplicationSystem::FlushPeriodic(const RepFrame& frame)
{
}

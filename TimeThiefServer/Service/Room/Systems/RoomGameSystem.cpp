#include "pch.h"
#include "RoomGameSystem.h"

/*------------------
   RoomGameSystem
------------------*/

bool RoomGameSystem::Init(Room* ownerRoom)
{
   if (ownerRoom == nullptr)
      return false;
   
   ownerRoom_ = ownerRoom;
   
   // if (!zoneSystem_.Init(ownerRoom))
   //    return false;
   
   return true;
}

void RoomGameSystem::Update(float deltaTime)
{
}

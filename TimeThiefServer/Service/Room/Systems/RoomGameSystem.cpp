#include "pch.h"
#include "RoomGameSystem.h"
#include "Data/GameDataManager.h"

/*------------------
   RoomGameSystem
------------------*/

bool RoomGameSystem::Init(Room* ownerRoom, const GameDataManager& gameDataManager)
{
   if (ownerRoom == nullptr)
      return false;
   
   ownerRoom_ = ownerRoom;
   gameDataManager_ = &gameDataManager;
   
   if (!zoneSystem_.Init(ownerRoom, ZoneBounds{}, gameDataManager_->GetZoneTable()))
      return false;
   
   return true;
}

void RoomGameSystem::Update(float deltaTime)
{
}

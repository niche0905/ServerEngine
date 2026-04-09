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
   
   // TEMP: 아래 ZoneBounds 값은 나중에 제대로 바꾸어야 함 (Map Data를 읽던, 하드 코딩으로 Map min, max를 넣던)
   if (!zoneSystem_.Init(ownerRoom, ZoneBounds{ SE::Math::Vector3{}, SE::Math::Vector3{150000, 150000, 0.0f} }, gameDataManager_->GetZoneTable()))
      return false;
   
   return true;
}

void RoomGameSystem::Update(float deltaTime)
{
}

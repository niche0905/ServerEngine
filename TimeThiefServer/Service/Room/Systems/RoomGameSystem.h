#pragma once
#include "Zone/ZoneSystem.h"

class GameDataManager;
class Room;

/*------------------
   RoomGameSystem
------------------*/
//
// RoomGameSystem은 Room 단위의 게임 진행 시스템들을 관리합니다.
//

class RoomGameSystem
{
public:
   RoomGameSystem() = default;
   
   bool Init(Room* ownerRoom, const GameDataManager& gameDataManager);
   void Update(float deltaTime);
   
   ZoneSystem& GetZoneSystem() { return zoneSystem_; }
   const ZoneSystem& GetZoneSystem() const { return zoneSystem_; }
   
private:
   Room*                   ownerRoom_ = nullptr;
   const GameDataManager*  gameDataManager_ = nullptr;
   
   ZoneSystem              zoneSystem_{};
    
};

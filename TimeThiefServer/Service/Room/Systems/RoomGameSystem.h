#pragma once
#include "Respawn/RespawnSystem.h"
#include "Zone/ZoneSystem.h"

struct GameConfig;
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
   
   bool Init(Room* ownerRoom, const GameDataManager& gameDataManager, const GameConfig& gameConfig);
   
   bool Start();
   void Update(float deltaTime);
   
public:
   ZoneSystem& GetZoneSystem() { return zoneSystem_; }
   const ZoneSystem& GetZoneSystem() const { return zoneSystem_; }
   
   RespawnSystem& GetRespawnSystem() { return respawnSystem_; }
   const RespawnSystem& GetRespawnSystem() const { return respawnSystem_; }
   
public:
   void OnPawnDeath(ObjectId pawnId);
   
private:
   Room*                   ownerRoom_ = nullptr;
   const GameDataManager*  gameDataManager_ = nullptr;
   
   ZoneSystem              zoneSystem_{};
   RespawnSystem           respawnSystem_{};
    
};

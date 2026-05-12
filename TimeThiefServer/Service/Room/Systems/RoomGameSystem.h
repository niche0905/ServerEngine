#pragma once
#include "Combat/CombatSystem.h"
#include "Drop/DropSystem.h"
#include "Loot/LootSystem.h"
#include "Replication/ReplicationSystem.h"
#include "Respawn/RespawnSystem.h"
#include "Store/StoreSystem.h"
#include "Upgrade/UpgradeSystem.h"
#include "Weapon/WeaponSystem.h"
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
   
   ReplicationSystem& GetReplicationSystem() { return replicationSystem_; }
   const ReplicationSystem& GetReplicationSystem() const { return replicationSystem_; }
   
   DropSystem& GetDropSystem() { return dropSystem_; }
   const DropSystem& GetDropSystem() const { return dropSystem_; }
   
   LootSystem& GetLootSystem() { return lootSystem_; }
   const LootSystem& GetLootSystem() const { return lootSystem_; }
   
   StoreSystem& GetStoreSystem() { return storeSystem_; }
   const StoreSystem& GetStoreSystem() const { return storeSystem_; }
   
   WeaponSystem& GetWeaponSystem() { return weaponSystem_; }
   const WeaponSystem& GetWeaponSystem() const { return weaponSystem_; }
   
   UpgradeSystem& GetUpgradeSystem() { return upgradeSystem_; }
   const UpgradeSystem& GetUpgradeSystem() const { return upgradeSystem_; }
   
   CombatSystem& GetCombatSystem() { return combatSystem_; }
   const CombatSystem& GetCombatSystem() const { return combatSystem_; }
   
public:
   void OnPawnDeath(ObjectId pawnId);
   
private:
   Room*                   ownerRoom_ = nullptr;
   const GameDataManager*  gameDataManager_ = nullptr;
   
   ZoneSystem              zoneSystem_{};
   RespawnSystem           respawnSystem_{};
   ReplicationSystem       replicationSystem_{};
   DropSystem              dropSystem_{};
   LootSystem              lootSystem_{};
   StoreSystem             storeSystem_{};
   WeaponSystem            weaponSystem_{};
   UpgradeSystem           upgradeSystem_{};
   CombatSystem            combatSystem_{};
    
};

#pragma once
#include "Data/Tables/ZoneTable.h"
#include "Tables/LootTableTypes.h"
#include "Tables/StoreEntryTable.h"
#include "Tables/WeaponTable.h"

struct ServerConfig;

/*-------------------
   GameDataManager
-------------------*/
//
// GameDataManager는 게임에서 사용되는 다양한 데이터들을 관리하는 클래스입니다.
//

class GameDataManager
{
public:
   bool Init(const ServerConfig& config);
   const ZoneTable& GetZoneTable() const { return zoneTable_; }
   const LootTable& GetLootTable() const { return lootTable_; }
   const StoreEntryTable& GetStoreEntryTable() const { return storeEntryTable_; }
   const WeaponTable& GetWeaponTable() const { return weaponTable_; }
   
private:
   ZoneTable                  zoneTable_;
   LootTable                  lootTable_;
   StoreEntryTable            storeEntryTable_;
   WeaponTable                weaponTable_;
   
};

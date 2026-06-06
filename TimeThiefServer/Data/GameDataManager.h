#pragma once
#include "Data/Tables/ZoneTable.h"
#include "Map/ServerMap.h"
#include "Placements/PlacementTypes.h"
#include "Tables/LootTableTypes.h"
#include "Tables/MonsterTemplateTable.h"
#include "Tables/NpcAiTable.h"
#include "Tables/PlayerSpawnTable.h"
#include "Tables/SkillTable.h"
#include "Tables/StoreEntryTable.h"
#include "Tables/UpgradeTable.h"
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
   const ServerMap& GetServerMap() const { return serverMap_; }
   const ZoneTable& GetZoneTable() const { return zoneTable_; }
   const LootTable& GetLootTable() const { return lootTable_; }
   const MonsterTemplateTable& GetMonsterTemplateTable() const { return monsterTemplateTable_; }
   const StoreEntryTable& GetStoreEntryTable() const { return storeEntryTable_; }
   const SkillTable& GetSkillTable() const { return skillTable_; }
   const WeaponTable& GetWeaponTable() const { return weaponTable_; }
   const UpgradeTable& GetUpgradeTable() const { return upgradeTable_; }
   const PlayerSpawnTable& GetPlayerSpawnTable() const { return playerSpawnTable_; }
   const NpcAiTable& GetNpcAiTable() const { return npcAiTable_; }
   const MapPlacementData& GetMapPlacementData() const { return mapPlacementData_; }
   
private:
   ServerMap                  serverMap_;
   ZoneTable                  zoneTable_;
   LootTable                  lootTable_;
   MonsterTemplateTable       monsterTemplateTable_;
   StoreEntryTable            storeEntryTable_;
   SkillTable                 skillTable_;
   WeaponTable                weaponTable_;
   UpgradeTable               upgradeTable_;
   PlayerSpawnTable           playerSpawnTable_;
   NpcAiTable                 npcAiTable_;
   MapPlacementData           mapPlacementData_;
   
};

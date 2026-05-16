#include "pch.h"
#include "GameDataManager.h"
#include "Loader/ServerMapLoader.h"
#include "Network/ServerConfig.h"
#include "Tables/LootTableJson.h"
#include "Tables/PlayerSpawnTableJson.h"
#include "Tables/StoreEntryTableJson.h"
#include "Tables/UpgradeTableJson.h"
#include "Tables/WeaponTableJson.h"
#include "Tables/ZoneTableJson.h"

/*-------------------
   GameDataManager
-------------------*/

bool GameDataManager::Init(const ServerConfig& config)
{
   std::string error;
   
   se::map::ServerMapLoader loader;
   se::map::LoadedMapData loadedMapData;

   if (not loader.LoadFromFile(config.dataFiles.mapFilePath, loadedMapData, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to Map Data: %hs\n", error.c_str());
      return false;
   }

   if (loadedMapData.colliders.empty()) {
      consoleLogger->Log(Color::Red, L"[GDM] No Colliders loaded: %hs\n", error.c_str());
      return false;
   }
   
   if (not serverMap_.BuildFromLoadedData(loadedMapData)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to build ServerMap from loaded data.\n");
      return false;
   }
   
   if (not serverMap_.LoadNavigation(config.dataFiles.navMeshFilePath)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load navigation data.\n");
      return false;
   }
   
   // consoleLogger->Log(Color::Blue, L"Map data loaded successfully. Collider count: %zu\n", loadedMapData.colliders.size());
   
   if (not ZoneTableJson::LoadFromFile(config.dataFiles.zoneTablePath, zoneTable_, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load ZoneTable: %hs\n", error.c_str());
      return false;
   }

   if (not LootTableJson::LoadFromFile(config.dataFiles.lootTablePath, lootTable_, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load LootTable: %hs\n", error.c_str());
      return false;
   }
   
   // debug log
   // consoleLogger->Log(Color::Green, L"[GDM] ZoneTable loaded successfully. Number of phases: %zu\n", zoneTable_.phases.size());
   // for (size_t i = 0; i < zoneTable_.phases.size(); i++) {
   //    const auto& phase = zoneTable_.phases[i];
   //    consoleLogger->Log(Color::Green, L"Phase %zu => radius: %.2f, damagePerSecond: %.2f, waitTimeSeconds: %.2f, shrinkTimeSeconds: %.2f\n",
   //       i + 1, phase.radius, phase.damagePerSecond, phase.waitTimeSeconds, phase.shrinkTimeSeconds);
   // }
   
   // debug log
   // if (lootTable_.IsValid()) {
   //    consoleLogger->Log(Color::Green, L"[GDM] LootTable loaded successfully.\n");
   // }
   
   if (!lootTable_.IsValid()) {
      consoleLogger->Log(Color::Red, L"[GDM] Loaded LootTable is invalid.\n");
      return false;
   }
   
   if (not StoreEntryTableJson::LoadFromFile(config.dataFiles.storeEntryTablePath, storeEntryTable_, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load StoreEntryTable: %hs\n", error.c_str());
      return false;
   }
   
   if (not WeaponTableJson::LoadFromFile(config.dataFiles.weaponTablePath, weaponTable_, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load WeaponTable: %hs\n", error.c_str());
      return false;
   }
   
   if (not UpgradeTableJson::LoadFromFile(config.dataFiles.weaponUpgradeTablePath, upgradeTable_.WeaponUpgradeTable, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load WeaponUpgradeTable: %hs\n", error.c_str());
      return false;
   }
   if (not UpgradeTableJson::LoadFromFile(config.dataFiles.statUpgradeTablePath, upgradeTable_.StatUpgradeTable, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load StatUpgradeTable: %hs\n", error.c_str());
      return false;
   }
   
   bool loadedPlayerSpawnTable = true;
   if (not PlayerSpawnTableJson::LoadFromFile(config.dataFiles.playerSpawnTablePath, playerSpawnTable_, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load PlayerSpawnTable: %hs\n", error.c_str());
      loadedPlayerSpawnTable = false;
   }
   if (config.game.testSpawnPoints or !loadedPlayerSpawnTable) {
      // 테스트용 스폰 포인트를 활성화 했을 경우, 혹은 파일에서 스폰 포인트를 불러오는데 실패했을 경우
      
      playerSpawnTable_.spawnPoints.clear();
      playerSpawnTable_.spawnPoints.reserve(10);
      
      for (int32 i = 0; i < 10; ++i) {
         playerSpawnTable_.spawnPoints.emplace_back(
            (0.0f + i * 200.0f), 0.0f, 100.0f
         );
      }
   }
   
   return true;
}

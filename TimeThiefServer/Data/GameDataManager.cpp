#include "pch.h"
#include "GameDataManager.h"
#include "Network/ServerConfig.h"
#include "Tables/LootTableJson.h"
#include "Tables/PlayerSpawnTableJson.h"
#include "Tables/ZoneTableJson.h"

/*-------------------
   GameDataManager
-------------------*/

bool GameDataManager::Init(const ServerConfig& config)
{
   std::string error;
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
   
   // TODO: Store Table 로드하기 (우선 json 파일 작성부터)
   // TODO: Weapon Table 로드하기 (우선 json 파일 작성부터)
   // TODO: Upgrade Table 로드하기 (우선 json 파일 작성부터)
   
   // TEMP: (파일 입출력이 아닌 코드로 무기 데이터 초기화)
   weaponTable_.tables[1] = {
      WeaponCommonStat{WeaponCategory::Rifle, WeaponFireType::HitScan, 12, 30, 0.1f, 2.0f, 10000.0f},
      RifleStat{}
   };
   weaponTable_.tables[2] = {
      WeaponCommonStat{WeaponCategory::Shotgun, WeaponFireType::HitScan, 8, 8, 60.0f / 110.0f, 2.0f, 3000.0f},
      ShotgunStat{12, 3.5f}
   };
   weaponTable_.tables[3] = {
      WeaponCommonStat{WeaponCategory::Launcher, WeaponFireType::Projectile, 80, 1, 60.0f / 48.0f, 3.0f, 200.0f},
      LauncherStat{2000.0f, 300.0f}
   };
   
   bool loadedPlayerSpawnTable = true;
   if (not PlayerSpawnTableJson::LoadFromFile(config.dataFiles.playerSpawnTablePath, playerSpawnTable_, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load PlayerSpawnTable: %hs\n", error.c_str());
      loadedPlayerSpawnTable = false;
   }
   if (config.game.testSpawnPoints or !loadedPlayerSpawnTable) {
      // 테스트용 스폰 포인트를 활성화 했을 경우, 혹은 파일에서 스폰 포인트를 불러오는데 실패했을 경우
      
      playerSpawnTable_.spawnPoints.clear();
      playerSpawnTable_.spawnPoints.reserve(20);
      
      for (int32 i = 0; i < 20; ++i) {
         playerSpawnTable_.spawnPoints.emplace_back(
            (0.0f + i * 200.0f), 0.0f, 100.0f
         );
      }
   }
   
   return true;
}

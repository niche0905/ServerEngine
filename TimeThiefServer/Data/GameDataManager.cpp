#include "pch.h"
#include "GameDataManager.h"
#include "Network/ServerConfig.h"
#include "Tables/LootTableJson.h"
#include "Tables/ZoneTableJson.h"

/*-------------------
   GameDataManager
-------------------*/

bool GameDataManager::Init(const ServerConfig& config)
{
   std::string error;
   if (not ZoneTableJson::LoadFromFile(config.dataFiles.zoneTablePath, zoneTable_, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load ZoneTable: %s\n", error.c_str());
      return false;
   }

   if (not LootTableJson::LoadFromFile(config.dataFiles.lootTablePath, lootTable_, &error)) {
      consoleLogger->Log(Color::Red, L"[GDM] Failed to load LootTable: %s\n", error.c_str());
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
      WeaponCommonStat{WeaponCategory::Rifle, WeaponFireType::HitScan, 12, 30, 0.1f, 1.5f, 10000.0f},
      RifleStat{}
   };
   weaponTable_.tables[2] = {
      WeaponCommonStat{WeaponCategory::Shotgun, WeaponFireType::HitScan, 8, 8, 1.0f, 2.5f, 3000.0f},
      ShotgunStat{12, 0.5f}
   };
   weaponTable_.tables[3] = {
      WeaponCommonStat{WeaponCategory::Launcher, WeaponFireType::Projectile, 80, 1, 1.5f, 3.0f, 200.0f},
      LauncherStat{500.0f, 20.0f}
   };
   
   // TEMP: (파일 입출력이 아닌 코드로 플레이어 스폰 정보 초기화)
   playerSpawnTable_.spawnPoints = {
      SE::Math::Vector3{ 41825.19f, -15198.09f, 92.29f },
      SE::Math::Vector3{ 38692.11f, 18581.00f, 92.13f },
      SE::Math::Vector3{ 20253.15f, 39826.38f, -127.84f },
      SE::Math::Vector3{ -20000.00f, 40000.00f, 92.13f },
      SE::Math::Vector3{ -40000.0f, 20000.0f, 92.29f },
      SE::Math::Vector3{ -39625.87f, -20191.79f, 92.29f },
      SE::Math::Vector3{ -21912.13f, -38632.58f, 92.13f },
      SE::Math::Vector3{ 20150.02f, -39015.13f, 728.11f }
   };
   
   return true;
}

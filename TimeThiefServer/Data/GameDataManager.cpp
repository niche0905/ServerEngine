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

   // TODO: LootTable json파일 작성하기
   // if (not LootTableJson::LoadFromFile(config.dataFiles.lootTablePath, lootTable_, &error)) {
   //    consoleLogger->Log(Color::Red, L"[GDM] Failed to load LootTable: %s\n", error.c_str());
   //    return false;
   // }
   
   // debug log
   // consoleLogger->Log(Color::Green, L"[GDM] ZoneTable loaded successfully. Number of phases: %zu\n", zoneTable_.phases.size());
   // for (size_t i = 0; i < zoneTable_.phases.size(); i++) {
   //    const auto& phase = zoneTable_.phases[i];
   //    consoleLogger->Log(Color::Green, L"Phase %zu => radius: %.2f, damagePerSecond: %.2f, waitTimeSeconds: %.2f, shrinkTimeSeconds: %.2f\n",
   //       i + 1, phase.radius, phase.damagePerSecond, phase.waitTimeSeconds, phase.shrinkTimeSeconds);
   // }
   
   return true;
}

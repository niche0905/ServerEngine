#include "pch.h"
#include "GameDataManager.h"
#include "Network/ServerConfig.h"
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
   
   return true;
}

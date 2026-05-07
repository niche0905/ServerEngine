#include "pch.h"
#include "ServerConfigReader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <json/json.h>
#include <filesystem>

/*-----------------------
   ServerConfigReader
-----------------------*/

bool ServerConfigReader::LoadFromFile(const std::filesystem::path& filePath)
{
   // debug log
   std::filesystem::path absPath = std::filesystem::absolute(filePath);
	consoleLogger->Log(Color::Blue, L"[Config] Try to access: %S\n", absPath.string().c_str());
   
   std::ifstream file{filePath};
   if (not file.is_open())
   {
	   consoleLogger->Log(Color::Red, L"[Config] Failed to open file: %S\n", absPath.string().c_str());
      return false;
   }
   
   std::stringstream buffer;
   buffer << file.rdbuf();
   
   const std::filesystem::path baseDir = absPath.parent_path();
   
   if (not ParseJsonText(buffer.str(), baseDir))
   {
      consoleLogger->Log(Color::Red, L"[Config] Failed to parse config file: %S\n", absPath.string().c_str());
      return false;
   }
   
   loadedFilePath_ = absPath;
   return true;
}

bool ServerConfigReader::ParseJsonText(const std::string& jsonText, const std::filesystem::path& baseDir)
{
   Json::CharReaderBuilder builder;
   Json::Value root;
   std::string errs;
   
   std::istringstream iss(jsonText);
   
   if (not Json::parseFromStream(builder, iss, &root, &errs))
   {
      consoleLogger->Log(Color::Red, L"[Config] JSON parsing error: %S\n", errs.c_str());
      return false;
   }
   
   ServerConfig newConfig{};
   
   // network
   if (root.isMember("network"))
   {
      const Json::Value& network = root["network"];
      
      if (network.isMember("bind_ip"))
         newConfig.network.bindIp = network["bind_ip"].asString();
      
      if (network.isMember("game_port"))
         newConfig.network.gamePort = network["game_port"].asInt();
      
      if (network.isMember("login_port"))
         newConfig.network.loginPort = network["login_port"].asInt();
   }
   
   // data files
   if (root.isMember("data_files"))
   {
      const Json::Value& dataFiles = root["data_files"];
      
      if (dataFiles.isMember("zone_table"))
         newConfig.dataFiles.zoneTablePath = ResolvePath(dataFiles["zone_table"].asString(), baseDir);
      
      if (dataFiles.isMember("loot_table"))
         newConfig.dataFiles.lootTablePath = ResolvePath(dataFiles["loot_table"].asString(), baseDir);

      if (dataFiles.isMember("weapon_upgrade_table"))
         newConfig.dataFiles.weaponUpgradeTablePath = ResolvePath(dataFiles["weapon_upgrade_table"].asString(), baseDir);
      
      if (dataFiles.isMember("stat_upgrade_table"))
         newConfig.dataFiles.statUpgradeTablePath = ResolvePath(dataFiles["stat_upgrade_table"].asString(), baseDir);
      
      if (dataFiles.isMember("player_spawn_table"))
         newConfig.dataFiles.playerSpawnTablePath = ResolvePath(dataFiles["player_spawn_table"].asString(), baseDir);
      
   }
   
   // game
   if (root.isMember("game"))
   {
      const Json::Value& game = root["game"];
      
      if (game.isMember("movement_update_hz"))
         newConfig.game.movementUpdateHz = game["movement_update_hz"].asInt();
      
      if (game.isMember("ping_interval_ms"))
         newConfig.game.pingIntervalMs = game["ping_interval_ms"].asInt();
      
      if (game.isMember("room_tick_interval_ms"))
         newConfig.game.roomTickIntervalMs = game["room_tick_interval_ms"].asInt();
      
      if (game.isMember("zone_damage_tick_interval"))
         newConfig.game.zoneDamageTickInterval = game["zone_damage_tick_interval"].asFloat();
      
      if (game.isMember("match_size")) {
         int32 matchSize = game["match_size"].asInt();
         newConfig.game.matchSize = std::min(8, std::max(2, matchSize));   // 최소 2명, 최대 8명으로 제한
      }
      
      if (game.isMember("test_spawn_points"))
         newConfig.game.testSpawnPoints = game["test_spawn_points"].asBool();
   }
   
   config_ = std::move(newConfig);
   return true;
}

std::filesystem::path ServerConfigReader::ResolvePath(const std::string& rawPath,
   const std::filesystem::path& baseDir) const
{
   if (rawPath.empty())
      return {};
   
   std::filesystem::path path{ rawPath };
   
   if (path.is_absolute())
      return path.lexically_normal();
   
   return (baseDir / path).lexically_normal();
}


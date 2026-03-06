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

ServerConfigReader g_ConfigReader;

bool ServerConfigReader::LoadFromFile(const std::string& filePath)
{
   // debug log
   std::filesystem::path p = std::filesystem::absolute(filePath);
	consoleLogger->Log(Color::Blue, L"[Config] Try to access: %S\n", p.string().c_str());
   
   std::ifstream file{filePath};
   if (not file.is_open())
   {
	   consoleLogger->Log(Color::Red, L"[Config] Failed to open file: %S\n", filePath.c_str());
      return false;
   }
   
   std::stringstream buffer;
   buffer << file.rdbuf();
   
   if (not ParseJsonText(buffer.str()))
   {
      consoleLogger->Log(Color::Red, L"[Config] Failed to parse config file: %S\n", filePath.c_str());
      return false;
   }
   
   loadedFilePath_ = filePath;
   return true;
}

bool ServerConfigReader::Reload()
{
   if (loadedFilePath_.empty()) return false;
   
   return LoadFromFile(loadedFilePath_);
}

bool ServerConfigReader::ParseJsonText(const std::string& jsonText)
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
   
   // network
   if (root.isMember("network"))
   {
      const Json::Value& network = root["network"];
      
      if (network.isMember("bind_ip"))
         config_.network.bindIp = network["bind_ip"].asString();
      
      if (network.isMember("game_port"))
         config_.network.gamePort = network["game_port"].asInt();
      
      if (network.isMember("login_port"))
         config_.network.loginPort = network["login_port"].asInt();
   }
   
   return true;
}


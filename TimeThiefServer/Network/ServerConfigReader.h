#pragma once
#include <string>
#include "ServerConfig.h"

/*-----------------------
   ServerConfigReader
-----------------------*/
//
// ServerConfigReader는 서버 설정 파일을 읽어와 ServerConfig 객체로 변환하는 역할을 합니다.
//

class ServerConfigReader
{
public:
   bool LoadFromFile(const std::string& filePath);
   bool Reload();
   
   const ServerConfig& Get() const { return config_; }
   
private:
   bool ParseJsonText(const std::string& jsonText);
   
private:
   std::string loadedFilePath_;
   ServerConfig config_;
    
};

extern ServerConfigReader g_ConfigReader;

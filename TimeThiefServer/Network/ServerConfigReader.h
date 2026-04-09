#pragma once
#include <string>
#include <filesystem>
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
   bool LoadFromFile(const std::filesystem::path& filePath);
   
   const ServerConfig& Get() const { return config_; }
   
private:
   bool ParseJsonText(const std::string& jsonText, const std::filesystem::path& baseDir);
   std::filesystem::path ResolvePath(const std::string& rawPath, const std::filesystem::path& baseDir) const;
   
private:
   std::filesystem::path      loadedFilePath_{};
   ServerConfig               config_{};
    
};

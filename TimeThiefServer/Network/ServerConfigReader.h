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

// TODO: TTSA 완성하고 제대로 의존성 제거 후 아래 전역 변수 정의 지우기
extern ServerConfigReader g_ConfigReader;

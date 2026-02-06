#pragma once
#include <string>

/*-----------------
   IConfigLoader
-----------------*/
//
// IConfigLoader는 설정 파일을 로드하는 인터페이스입니다.
//

namespace SE::Config
{
    class ConfigDocument
    {
    public:
        ConfigDocument() = default;
        bool IsValid() const { return valid_; }
        
        void _SetValid(bool v) { valid_ = v; }
        
    private:
        bool valid_{ false };
    };
    
    class IConfigLoader
    {
    public:
        virtual ~IConfigLoader() = default;
        
        // 파일 경로로 부터 설정 문서를 로드합니다.
        virtual bool LoadFromFile(const std::string& filepath, ConfigDocument& outDoc) = 0;
        
        // 문자열로 부터 설정 문서를 로드합니다.
        virtual bool LoadFromString(const std::string& text, ConfigDocument& outDoc) = 0;
        
        // 마지막 에러 메시지를 반환합니다. (로깅용)
        virtual const char* GetLastError() const = 0;
        
    };
    
}

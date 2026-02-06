#pragma once
#include <string>
#include <json/json.h>

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
        const Json::Value& Root() const { return root_; }
        
        void _SetValid(bool v) { valid_ = v; }
        Json::Value& _MutableRoot() { return root_; }
        
        void _SetError(std::string msg) { error_ = std::move(msg); }
        const std::string& GetError() const { return error_; }
        
        void Clear()
        {
            valid_ = false;
            root_.clear();
            error_.clear();
        }
        
    private:
        bool valid_{ false };
        Json::Value root_;
        std::string error_;
        
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

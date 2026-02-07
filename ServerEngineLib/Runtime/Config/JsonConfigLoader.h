#pragma once
#include "IConfigLoader.h"

/*--------------------
   JsonConfigLoader
--------------------*/
//
// JsonConfigLoader는 JSON 형식의 설정 파일을 로드하는 구현체입니다.
//

namespace SE::Config
{
    class JsonConfigLoader final : public IConfigLoader
    {
    public:
        bool LoadFromFile(const std::string& filepath, ConfigDocument& outDoc) override;
        
        bool LoadFromString(const std::string& text, ConfigDocument& outDoc) override;
        
        const char* GetLastError() const override { return lastError_.c_str(); }
        
    private:
        std::string lastError_;
    
    };

}

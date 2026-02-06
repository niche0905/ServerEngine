#pragma once
#include "Content/Enum/LootTypes.h"
#include "Runtime/Config/IConfigLoader.h"
#include <string>
#include <unordered_map>

struct LootTableDef
{
    int32 tableId{0};
};

/*--------------------
   LootTableService
--------------------*/
//
// LootTableService는 루팅 테이블을 관리합니다.
//

class LootTableService
{
public:
    explicit LootTableService(SE::Config::IConfigLoader& loader)
        : configLoader_(loader)
    {}
    
    bool LoadFromFile(const std::string& filepath);
    bool Reload();
    
    LootBundle Roll(int32 tableId, uint32 rngSeed, const LootRollContext& ctx) const;
    
    bool HasTable(int32 tableId) const;
    
private:
    SE::Config::IConfigLoader& configLoader_;
    std::string lastPath;
    
    std::unordered_map<int32, LootTableDef> tables_;
    
};
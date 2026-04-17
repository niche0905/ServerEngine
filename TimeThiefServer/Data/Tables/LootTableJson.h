#pragma once
#include <filesystem>
#include <string>
#include "LootTableTypes.h"

namespace LootTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, LootTable& outZoneTable, std::string* outError = nullptr);
    bool SaveToFile(const std::filesystem::path& filePath, const LootTable& table, std::string* outError = nullptr);
    
    bool Validate(const LootTable& table, std::string* outError = nullptr);
    
}

#pragma once
#include <filesystem>
#include <string>
#include "ZoneTable.h"

namespace ZoneTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, ZoneTable& outZoneTable, std::string* outError = nullptr);
    bool SaveToFile(const std::filesystem::path& filePath, const ZoneTable& table, std::string* outError = nullptr);
    
    bool Validate(const ZoneTable& table, std::string* outError = nullptr);
    
}

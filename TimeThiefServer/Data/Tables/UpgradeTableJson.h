#pragma once
#include <filesystem>
#include <string>
#include "UpgradeTable.h"

namespace UpgradeTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, WeaponUpgradeTable& outTable, std::string* outError = nullptr);
    bool LoadFromFile(const std::filesystem::path& filePath, StatUpgradeTable& outTable, std::string* outError = nullptr);
    
}

#pragma once
#include <filesystem>
#include <string>
#include "WeaponTable.h"

namespace WeaponTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, WeaponTable& outTable, std::string* outError = nullptr);
    
}

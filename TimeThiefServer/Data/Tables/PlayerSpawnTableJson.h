#pragma once
#include <filesystem>
#include <string>
#include "PlayerSpawnTable.h"

namespace PlayerSpawnTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, PlayerSpawnTable& outTable, std::string* outError = nullptr);
    
}

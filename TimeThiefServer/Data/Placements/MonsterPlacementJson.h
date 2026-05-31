#pragma once
#include <filesystem>
#include <string>
#include "PlacementTypes.h"

namespace MonsterPlacementJson
{
   bool LoadFromFile(const std::filesystem::path& filePath, MonsterPlacementData& outData, std::string* outError = nullptr);
}

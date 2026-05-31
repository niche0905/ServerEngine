#pragma once
#include <filesystem>
#include <string>
#include "PlacementTypes.h"

namespace InteractionPlacementJson
{
   bool LoadFromFile(const std::filesystem::path& filePath, InteractionPlacementData& outData, std::string* outError = nullptr);
}

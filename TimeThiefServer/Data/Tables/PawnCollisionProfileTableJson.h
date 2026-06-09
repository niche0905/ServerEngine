#pragma once
#include <filesystem>
#include <string>
#include "PawnCollisionProfileTable.h"

namespace PawnCollisionProfileTableJson
{
   bool LoadFromFile(const std::filesystem::path& filePath, PawnCollisionProfileTable& outTable, std::string* outError);
}

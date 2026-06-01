#pragma once
#include <filesystem>
#include <string>
#include "MonsterTemplateTable.h"

namespace MonsterTemplateTableJson
{
   bool LoadFromFile(const std::filesystem::path& filePath, MonsterTemplateTable& outTable, std::string* outError = nullptr);
}

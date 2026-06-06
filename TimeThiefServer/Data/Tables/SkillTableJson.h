#pragma once
#include <filesystem>
#include <string>
#include "SkillTable.h"

namespace SkillTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, SkillTable& outTable, std::string* outError = nullptr);
}

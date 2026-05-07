#pragma once
#include <filesystem>
#include <string>
#include "StoreEntryTable.h"

namespace StoreEntryTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, StoreEntryTable& outTable, std::string* outError = nullptr);
    
}

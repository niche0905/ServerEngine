#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include "Data/Map/ServerMapFormat.h"


namespace se::map
{
    struct LoadedMapData
    {
        MapHeader header;
        std::vector<ColliderData> colliders;
    };
    
    class ServerMapLoader
    {
    public:
        bool LoadFromFile(const std::filesystem::path& filePath, LoadedMapData& outMapData, std::string* outError = nullptr);
    
    };
}



#pragma once

#include <vector>
#include <string>
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
        bool LoadFromFile(const std::string& filePath, LoadedMapData& outMapData);
    
    };
}



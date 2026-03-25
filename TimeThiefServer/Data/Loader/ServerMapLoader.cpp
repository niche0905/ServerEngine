#include "pch.h"
#include "ServerMapLoader.h"

#include <fstream>


namespace se::map
{
    bool ServerMapLoader::LoadFromFile(const std::string& filePath, LoadedMapData& outMapData)
    {
        outMapData = {};
        
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (not file.is_open()) {
            return false;
        }
        
        const std::streamsize fileSize = file.tellg();
        if (fileSize < static_cast<std::streamsize>(sizeof(MapHeader))) {
            return false;
        }
        
        file.seekg(0, std::ios::beg);
        
        MapHeader header{};
        file.read(reinterpret_cast<char*>(&header), sizeof(MapHeader));
        if (!file) {
            return false;
        }
        
        if (header.magic != kServerMapMagic) {
            return false;
        }
        
        if (header.version != kServerMapVersion) {
            return false;
        }
        
        const uint64_t expectedSize = static_cast<uint64_t>(sizeof(MapHeader)) + static_cast<uint64_t>(header.colliderCount) * sizeof(ColliderData);
        if (expectedSize > static_cast<uint64_t>(fileSize)) {
            return false;
        }
        
        outMapData.header = header;
        outMapData.colliders.resize(header.colliderCount);
        
        if (header.colliderCount > 0) {
            
            file.read(reinterpret_cast<char*>(outMapData.colliders.data()), static_cast<std::streamsize>(sizeof(ColliderData) * header.colliderCount));
            
            if (!file) {
                return false;
            }
        }
        
        return true;
    }
}

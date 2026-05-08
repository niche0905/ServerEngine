#include "pch.h"
#include "ServerMapLoader.h"
#include <filesystem>
#include <fstream>


namespace se::map
{
    bool ServerMapLoader::LoadFromFile(const std::filesystem::path& filePath, LoadedMapData& outMapData, std::string* outError)
    {
        outMapData = {};
        
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (not file.is_open()) {
            if (outError) *outError = "Failed to open file";
            return false;
        }
        
        const std::streamsize fileSize = file.tellg();
        if (fileSize < static_cast<std::streamsize>(sizeof(MapHeader))) {
            if (outError) *outError = "File too small to contain valid map data";
            return false;
        }
        
        file.seekg(0, std::ios::beg);
        
        MapHeader header{};
        file.read(reinterpret_cast<char*>(&header), sizeof(MapHeader));
        if (!file) {
            if (outError) *outError = "Failed to read map header";
            return false;
        }
        
        if (header.magic != kServerMapMagic) {
            if (outError) *outError = "Invalid file format (magic number mismatch)";
            return false;
        }
        
        if (header.version != kServerMapVersion) {
            if (outError) *outError = "Invalid file format (version number mismatch)";
            return false;
        }
        
        const uint64_t expectedSize = static_cast<uint64_t>(sizeof(MapHeader)) + static_cast<uint64_t>(header.colliderCount) * sizeof(ColliderData);
        if (expectedSize > static_cast<uint64_t>(fileSize)) {
            if (outError) *outError = "File size does not match expected size based on header information";
            return false;
        }
        
        outMapData.header = header;
        outMapData.colliders.resize(header.colliderCount);
        
        if (header.colliderCount > 0) {
            
            file.read(reinterpret_cast<char*>(outMapData.colliders.data()), static_cast<std::streamsize>(sizeof(ColliderData) * header.colliderCount));
            
            if (!file) {
                if (outError) *outError = "Failed to read collider data";
                return false;
            }
        }
        
        return true;
    }
}

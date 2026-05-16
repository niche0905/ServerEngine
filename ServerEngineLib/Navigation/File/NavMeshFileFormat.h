#pragma once
#include <vector>
#include <recastnavigation/DetourNavMesh.h>

namespace SE::Navigation
{
    constexpr uint32 kNavMeshFileMagic = 0x54414E53; // 'SNAV'
    constexpr uint32 kNavMeshFileVersion = 1;
    
    struct NavMeshSetHeader
    {
        uint32 magic = kNavMeshFileMagic;
        uint32 version = kNavMeshFileVersion;
        uint32 tileCount = 0;
        dtNavMeshParams params{};
    };
    
    struct NavMeshTileHeader
    {
        uint64 tileRef = 0;
        uint32 dataSize = 0;
    };
    
    struct NavMeshTileBlob
    {
        NavMeshTileHeader header;
        std::vector<unsigned char> data;
    };
    
    struct NavMeshFileData
    {
        NavMeshSetHeader header;
        std::vector<NavMeshTileBlob> tiles;
    };
    
}

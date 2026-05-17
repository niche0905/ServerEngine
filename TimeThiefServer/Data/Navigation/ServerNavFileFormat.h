#pragma once
#include "Utils/Types.h"
#include "ThirdParty/UEDetour/DetourNavMesh.h"

namespace SE::Nav
{
    constexpr uint32 kServerNavMagic = 0x4D56414E; // 'NAVM'
    constexpr uint32 kServerNavVersion = 1;

    struct ServerNavBinaryHeader
    {
        uint32 magic = kServerNavMagic;
        uint32 version = kServerNavVersion;

        float orig[3] = {};
        float tileWidth = 0.0f;
        float tileHeight = 0.0f;

        int32 maxTiles = 0;
        int32 maxPolys = 0;

        int32 tileCount = 0;

        uint32 dtTileRefSize = sizeof(dtTileRef);
        uint32 dtNavMeshParamsSize = sizeof(dtNavMeshParams);
    };

    struct ServerNavTileHeader
    {
        dtTileRef tileRef = 0;
        uint32 tileDataSize = 0;
    };
}

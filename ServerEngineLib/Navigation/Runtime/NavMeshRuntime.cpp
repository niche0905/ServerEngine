#include "pch.h"
#include "NavMeshRuntime.h"
#include "Navigation/Runtime/NavMeshRuntime.h"
#include "Navigation/File/NavMeshFileReader.h"
#include "Navigation/Runtime/NavMeshLoader.h"

/*------------------
   NavMeshRuntime
------------------*/

namespace SE::Navigation
{
    static dtNavMeshParams ToDtNavMeshParams(const NavMeshFileParams& src)
    {
        dtNavMeshParams dst{};

        dst.orig[0] = src.orig[0];
        dst.orig[1] = src.orig[1];
        dst.orig[2] = src.orig[2];

        dst.tileWidth = src.tileWidth;
        dst.tileHeight = src.tileHeight;
        dst.maxTiles = src.maxTiles;
        dst.maxPolys = src.maxPolys;

        return dst;
    }
    
    bool NavMeshRuntime::LoadFromFile(const std::filesystem::path& filePath)
    {
        NavMeshFileData data;
        if (!NavMeshFileReader::ReadFromFile(filePath, data))
            return false;
        
        auto navMesh = std::unique_ptr<dtNavMesh, DtNavMeshDeleter>(dtAllocNavMesh());
        if (!navMesh)
            return false;
        
        dtNavMeshParams params = ToDtNavMeshParams(data.header.params);

        dtStatus status = navMesh->init(&params);
        if (dtStatusFailed(status))
            return false;

        for (auto& tile : data.tiles)
        {
            unsigned char* tileData = static_cast<unsigned char*>(dtAlloc(tile.header.dataSize, DT_ALLOC_PERM));
            if (!tileData)
                return false;

            std::memcpy(tileData, tile.data.data(), tile.header.dataSize);

            dtStatus tileStatus = navMesh->addTile(
                tileData,
                tile.header.dataSize,
                DT_TILE_FREE_DATA,
                tile.header.tileRef,
                nullptr
            );

            if (dtStatusFailed(tileStatus))
            {
                dtFree(tileData);
                return false;
            }
        }

        fileData_ = std::move(data);
        navMesh_ = std::move(navMesh);
        
        return true;
    }
}

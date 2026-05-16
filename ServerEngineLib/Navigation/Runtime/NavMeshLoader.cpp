#include "pch.h"
#include "NavMeshLoader.h"
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourAlloc.h>

/*-----------------
   NavMeshLoader
-----------------*/

namespace
{
    dtNavMeshParams ToDtNavMeshParams(const SE::Navigation::NavMeshFileParams& src)
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
}

namespace SE::Navigation
{
    void DtNavMeshDeleter::operator()(dtNavMesh* ptr) const
    {
        if (ptr) {
            dtFreeNavMesh(ptr);
        }
    }

    std::unique_ptr<dtNavMesh, DtNavMeshDeleter> NavMeshLoader::CreateNavMeshFromFile(
    const NavMeshFileData& fileData)
    {
        dtNavMesh* rawNavMesh = dtAllocNavMesh();
        if (!rawNavMesh) {
            return nullptr;
        }

        std::unique_ptr<dtNavMesh, DtNavMeshDeleter> navMesh(rawNavMesh);

        dtNavMeshParams params = ToDtNavMeshParams(fileData.header.params);

        if (dtStatusFailed(navMesh->init(&params))) {
            return nullptr;
        }
        
        for (const NavMeshTileBlob& tileBlob : fileData.tiles) {
            if (tileBlob.header.dataSize == 0 || tileBlob.data.empty()) {
                return nullptr;
            }
        
            unsigned char* tileData = static_cast<unsigned char*>(
                dtAlloc(tileBlob.header.dataSize, DT_ALLOC_PERM)
            );
            
            if (!tileData) {
                return nullptr;
            }
            
            std::memcpy(tileData, tileBlob.data.data(), tileBlob.header.dataSize);
            
            dtTileRef resultRef = 0;
            const dtStatus status = navMesh->addTile(
                tileData,
                static_cast<int>(tileBlob.header.dataSize),
                DT_TILE_FREE_DATA,
                static_cast<dtTileRef>(tileBlob.header.tileRef),
                &resultRef
            );
            
            if (dtStatusFailed(status)) {
                dtFree(tileData);
                return nullptr;
            }
        }
        
        return navMesh;
    }
}

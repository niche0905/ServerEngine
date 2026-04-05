#include "pch.h"
#include "NavMeshLoader.h"
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourAlloc.h>

/*-----------------
   NavMeshLoader
-----------------*/

namespace SE::Navigation
{
    void DtNavMeshDeleter::operator()(dtNavMesh* ptr) const
    {
        if (ptr) {
            dtFreeNavMesh(ptr);
        }
    }

    std::unique_ptr<dtNavMesh, DtNavMeshDeleter> NavMeshLoader::CreateNavMeshFromFile(const NavMeshFileData& fileData)
    {
        dtNavMesh* navMesh = dtAllocNavMesh();
        if (!navMesh) {
            return nullptr;  // 메모리 할당 실패
        }

        if (dtStatusFailed(navMesh->init(&fileData.header.params))) {
            dtFreeNavMesh(navMesh);
            return nullptr;  // 네비게이션 메시 초기화 실패
        }
        
        for (const NavMeshTileBlob& tileBlob : fileData.tiles) {
            
            unsigned char* tileData = static_cast<unsigned char*>(dtAlloc(tileBlob.header.dataSize, DT_ALLOC_PERM));
            
            if (!tileData) {
                dtFreeNavMesh(navMesh);
                return nullptr;  // 메모리 할당 실패
            }
            
            std::memcpy(tileData, tileBlob.data.data(), tileBlob.header.dataSize);
            
            dtTileRef resultRef = 0;
            const dtStatus status = navMesh->addTile(tileData, static_cast<int>(tileBlob.header.dataSize), 
                DT_TILE_FREE_DATA, static_cast<dtTileRef>(tileBlob.header.tileRef), &resultRef);
            
            if (dtStatusFailed(status)) {
                dtFree(tileData);
                dtFreeNavMesh(navMesh);
                return nullptr;  // 타일 추가 실패
            }
        }
        
        return std::unique_ptr<dtNavMesh, DtNavMeshDeleter>(navMesh);
    }
}

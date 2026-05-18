#include "pch.h"
#include "ServerNavigation.h"
#include "ServerNavFileFormat.h"
#include <fstream>
#include <memory>
#include <cfloat>
#include "ThirdParty/UEDetour/DetourAlloc.h"
#include "ThirdParty/UEDetour/DetourNavMesh.h"
#include "ThirdParty/UEDetour/DetourNavMeshQuery.h"

/*---------------------
   ServerNavigation
---------------------*/

namespace SE::Nav
{
    ServerNavigation::~ServerNavigation()
    {
        Release();
    }

    bool ServerNavigation::LoadFromFile(const std::filesystem::path& filePath)
    {
        // consoleLogger->Log(Color::Cyan, L"sizeof(dtReal)=%zu\n", sizeof(dtReal));
        // consoleLogger->Log(Color::Cyan, L"sizeof(dtMeshHeader)=%zu\n", sizeof(dtMeshHeader));
        // consoleLogger->Log(Color::Cyan, L"sizeof(dtPoly)=%zu\n", sizeof(dtPoly));
        // consoleLogger->Log(Color::Cyan, L"sizeof(dtLink)=%zu\n", sizeof(dtLink));
        // consoleLogger->Log(Color::Cyan, L"sizeof(dtPolyDetail)=%zu\n", sizeof(dtPolyDetail));
        // consoleLogger->Log(Color::Cyan, L"sizeof(dtBVNode)=%zu\n", sizeof(dtBVNode));
        // consoleLogger->Log(Color::Cyan, L"sizeof(dtOffMeshConnection)=%zu\n", sizeof(dtOffMeshConnection));
        //
        // consoleLogger->Log(Color::Cyan, L"DT_LARGE_WORLD_COORDINATES_DISABLED=%d\n", DT_LARGE_WORLD_COORDINATES_DISABLED);
        // consoleLogger->Log(Color::Cyan, L"WITH_NAVMESH_SEGMENT_LINKS=%d\n", WITH_NAVMESH_SEGMENT_LINKS);
        // consoleLogger->Log(Color::Cyan, L"WITH_NAVMESH_CLUSTER_LINKS=%d\n", WITH_NAVMESH_CLUSTER_LINKS);
        
        Release();
        
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
            return false;
        
        ServerNavBinaryHeader header{};
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        
        if (!file)
            return false;
        
        if (header.magic != kServerNavMagic)
            return false;
        
        if (header.version != kServerNavVersion)
            return false;
        
        if (header.dtTileRefSize != sizeof(dtTileRef))
            return false;
        
        if (header.dtNavMeshParamsSize != sizeof(dtNavMeshParams))
            return false;
        
        dtNavMeshParams params{};
        params.orig[0] = header.orig[0];
        params.orig[1] = header.orig[1];
        params.orig[2] = header.orig[2];
        params.tileWidth = header.tileWidth;
        params.tileHeight = header.tileHeight;
        params.maxTiles = header.maxTiles;
        params.maxPolys = header.maxPolys;
        
        navMesh_ = dtAllocNavMesh();
        if (!navMesh_)
            return false;
        
        dtStatus status = navMesh_->init(&params);
        if (dtStatusFailed(status)) {
            Release();
            return false;
        }
        
        for (int32 i = 0; i < header.tileCount; ++i) {
            
            ServerNavTileHeader tileHeader{};
            file.read(reinterpret_cast<char*>(&tileHeader), sizeof(tileHeader));
            
            if (!file or tileHeader.tileDataSize == 0) {
                Release();
                return false;
            }
            
            unsigned char* tileData = static_cast<unsigned char*>(dtAlloc(tileHeader.tileDataSize, DT_ALLOC_PERM_TILE_DATA));
            
            if (!tileData) {
                Release();
                return false;
            }
            
            file.read(reinterpret_cast<char*>(tileData), tileHeader.tileDataSize);
            
            if (!file) {
                dtFree(tileData, DT_ALLOC_PERM_TILE_DATA);
                Release();
                return false;
            }
            
            dtTileRef resultRef = 0;
            status = navMesh_->addTile(tileData, static_cast<int>(tileHeader.tileDataSize), DT_TILE_FREE_DATA, tileHeader.tileRef, &resultRef);
            
            if (dtStatusFailed(status) || resultRef == 0) {
                dtFree(tileData, DT_ALLOC_PERM_TILE_DATA);
                Release();
                return false;
            }
        }
        
        navQuery_ = dtAllocNavMeshQuery();
        if (!navQuery_) {
            Release();
            return false;
        }
        
        status = navQuery_->init(navMesh_, maxSearchNodes_);
        if (dtStatusFailed(status)) {
            Release();
            return false;
        }
        
        filter_ = new dtQueryFilter();
        
        // consoleLogger->Log(Color::Blue, L"[Navigation] Successfully loaded navigation mesh from file: %S\n", filePath.string().c_str());
        // consoleLogger->Log(Color::Blue, L"[Navigation] Loaded. tiles=%d, maxTiles=%d, maxPolys=%d\n", header.tileCount, header.maxTiles, header.maxPolys);
        
        // DebugPrintNavMeshBounds();
        
        return true;
    }

    bool ServerNavigation::FindNearestPoly(const SE::Math::Vector3& pos, const SE::Math::Vector3& halfExtents,
        dtPolyRef& outRef, SE::Math::Vector3& outNearest) const
    {
        if (!IsLoaded())
            return false;
        
        dtReal p[3];
        dtReal e[3];
        dtReal nearest[3];
        
        ToDetour(pos, p);
        ToDetourExtents(halfExtents, e);
        
        outRef = 0;
        
        const dtStatus status = navQuery_->findNearestPoly(p, e, filter_, &outRef, nearest, nullptr);
        
        if (dtStatusFailed(status) || outRef == 0)
            return false;
        
        outNearest = FromDetour(nearest);
        return true;
    }

    bool ServerNavigation::FindPath(const SE::Math::Vector3& start, const SE::Math::Vector3& end,
        std::vector<SE::Math::Vector3>& outPath) const
    {
        outPath.clear();
        
        if (!IsLoaded())
            return false;
        
        constexpr SE::Math::Vector3 halfExtents{300.0f, 300.0f, 300.0f};
        
        dtPolyRef startRef = 0;
        dtPolyRef endRef = 0;
        
        SE::Math::Vector3 nearestStart;
        SE::Math::Vector3 nearestEnd;
        
        if (!FindNearestPoly(start, halfExtents, startRef, nearestStart))
            return false;

        if (!FindNearestPoly(end, halfExtents, endRef, nearestEnd))
            return false;
        
        dtReal detourStart[3];
        dtReal detourEnd[3];
        
        ToDetour(nearestStart, detourStart);
        ToDetour(nearestEnd, detourEnd);
        
        dtQueryResult pathResult;
        pathResult.reserve(maxPathPolys_);

        dtReal totalCost = 0.0;

        constexpr dtReal CostLimit = static_cast<dtReal>(FLT_MAX);
        
        dtStatus status = navQuery_->findPath(startRef, endRef, detourStart, detourEnd,
                                                CostLimit, filter_, pathResult, &totalCost);
        
        if (dtStatusFailed(status) || pathResult.size() <= 0)
            return false;
        
        std::vector<dtPolyRef> polys(pathResult.size());
        pathResult.copyRefs(polys.data(), static_cast<int>(polys.size()));

        dtQueryResult straightResult;
        straightResult.reserve(maxStraightPath_);
        
        status = navQuery_->findStraightPath(detourStart, detourEnd, polys.data(), 
                                        static_cast<int>(polys.size()), straightResult, 0);
        
        if (dtStatusFailed(status) || straightResult.size() <= 0)
            return false;
        
        outPath.reserve(straightResult.size());

        for (int i = 0; i < straightResult.size(); ++i) {
            outPath.push_back(FromDetour(straightResult.getPos(i)));
        }
        
        return true;
    }

    bool ServerNavigation::DebugValidatePoint(const SE::Math::Vector3& pos) const
    {
        if (!IsLoaded())
            return false;
        
        dtPolyRef ref = 0;
        SE::Math::Vector3 nearest;
        
        constexpr SE::Math::Vector3 halfExtents{100.0f, 100.0f, 300.0f};
        
        if (!FindNearestPoly(pos, halfExtents, ref, nearest)) {
            consoleLogger->Log(Color::Red, L"[Navigation] FindNearestPoly failed. pos=(%.2f, %.2f, %.2f)\n", pos.x, pos.y, pos.z);
            return false;
        }
        
        consoleLogger->Log(Color::Green, L"[Navigation] FindNearestPoly success. ref=%llu nearest=(%.2f, %.2f, %.2f)\n", static_cast<unsigned long long>(ref), nearest.x, nearest.y, nearest.z);
        return true;
    }

    void ServerNavigation::DebugPrintNavMeshBounds() const
    {
        if (navMesh_ == nullptr) {
            consoleLogger->Log(Color::Red, L"[NavMesh] navMesh_ is null\n");
            return;
        }

        const dtNavMeshParams* params = navMesh_->getParams();
        consoleLogger->Log(Color::Cyan,
            L"[NavMesh] orig=(%.2f %.2f %.2f), tileWidth=%.2f, tileHeight=%.2f, maxTiles=%d, maxPolys=%d\n",
            (double)params->orig[0],
            (double)params->orig[1],
            (double)params->orig[2],
            (double)params->tileWidth,
            (double)params->tileHeight,
            params->maxTiles,
            params->maxPolys);

        // for (int i = 0; i < navMesh_->getMaxTiles(); ++i) {
        //     const dtMeshTile* tile = navMesh_->getTile(i);
        //     if (tile == nullptr || tile->header == nullptr)
        //         continue;
        //
        //     const dtMeshHeader* h = tile->header;
        //
        //     consoleLogger->Log(Color::Cyan,
        //         L"[NavMeshTile %d] bmin=(%.2f %.2f %.2f), bmax=(%.2f %.2f %.2f), polys=%d, verts=%d\n",
        //         i,
        //         (double)h->bmin[0],
        //         (double)h->bmin[1],
        //         (double)h->bmin[2],
        //         (double)h->bmax[0],
        //         (double)h->bmax[1],
        //         (double)h->bmax[2],
        //         h->polyCount,
        //         h->vertCount);
        // }
    }

    void ServerNavigation::DebugFindTilesAround(const SE::Math::Vector3& serverPos) const
    {
        dtReal p[3];
        ToDetour(serverPos, p);

        consoleLogger->Log(Color::Cyan,
            L"[DebugFindTilesAround] server=(%.2f %.2f %.2f), detour=(%.2f %.2f %.2f)\n",
            serverPos.x, serverPos.y, serverPos.z,
            (double)p[0], (double)p[1], (double)p[2]);

        // for (int i = 0; i < navMesh_->getMaxTiles(); ++i) {
        //     const dtMeshTile* tile = navMesh_->getTile(i);
        //     if (tile == nullptr || tile->header == nullptr)
        //         continue;
        //
        //     const dtMeshHeader* h = tile->header;
        //
        //     const bool nearX = p[0] >= h->bmin[0] - 1000.0 && p[0] <= h->bmax[0] + 1000.0;
        //     const bool nearZ = p[2] >= h->bmin[2] - 1000.0 && p[2] <= h->bmax[2] + 1000.0;
        //
        //     if (nearX && nearZ) {
        //         consoleLogger->Log(Color::Yellow,
        //             L"[NearTile %d] bmin=(%.2f %.2f %.2f), bmax=(%.2f %.2f %.2f), polys=%d\n",
        //             i,
        //             (double)h->bmin[0], (double)h->bmin[1], (double)h->bmin[2],
        //             (double)h->bmax[0], (double)h->bmax[1], (double)h->bmax[2],
        //             h->polyCount);
        //     }
        // }
    }

    void ServerNavigation::Release()
    {
        if (filter_) {
            delete filter_;
            filter_ = nullptr;
        }
        
        if (navQuery_) {
            dtFreeNavMeshQuery(navQuery_);
            navQuery_ = nullptr;
        }
        
        if (navMesh_) {
            dtFreeNavMesh(navMesh_);
            navMesh_ = nullptr;
        }
    }

    void ServerNavigation::ToDetour(const SE::Math::Vector3& in, dtReal out[3])
    {
        // Server/UE: X, Y, Z(up)
        // Detour:    X, Y(up), Z
        out[0] = static_cast<dtReal>(in.x);
        out[1] = static_cast<dtReal>(in.z);
        out[2] = static_cast<dtReal>(in.y);
    }
    
    void ServerNavigation::ToDetourExtents(const SE::Math::Vector3& in, dtReal out[3])
    {
        out[0] = static_cast<dtReal>(in.x);
        out[1] = static_cast<dtReal>(in.z);
        out[2] = static_cast<dtReal>(in.y);
    }

    SE::Math::Vector3 ServerNavigation::FromDetour(const dtReal in[3])
    {
        return SE::Math::Vector3(
            static_cast<float>(in[0]),
            static_cast<float>(in[2]),
            static_cast<float>(in[1]));
    }
}

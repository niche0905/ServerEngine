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
    ServerNavigation::QueryContext::~QueryContext()
    {
        Release();
    }

    bool ServerNavigation::QueryContext::Init(const ServerNavigation& navigation)
    {
        Release();

        if (!navigation.IsLoaded())
            return false;

        navQuery_ = dtAllocNavMeshQuery();
        if (!navQuery_) {
            Release();
            return false;
        }

        const dtStatus status = navQuery_->init(navigation.navMesh_, navigation.maxSearchNodes_);
        if (dtStatusFailed(status)) {
            Release();
            return false;
        }

        filter_ = new dtQueryFilter();
        if (!filter_) {
            Release();
            return false;
        }

        return true;
    }

    void ServerNavigation::QueryContext::Release()
    {
        if (filter_) {
            delete filter_;
            filter_ = nullptr;
        }

        if (navQuery_) {
            dtFreeNavMeshQuery(navQuery_);
            navQuery_ = nullptr;
        }
    }

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
        params.walkableHeight = header.walkableHeight;
        params.walkableRadius = header.walkableRadius;
        params.walkableClimb = header.walkableClimb;
        for (int i = 0; i < DT_RESOLUTION_COUNT; ++i) {
            params.resolutionParams[i] = header.resolutionParams[i];
        }
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
            // status = navMesh_->addTile(tileData, static_cast<int>(tileHeader.tileDataSize), DT_TILE_FREE_DATA, tileHeader.tileRef, &resultRef);
            status = navMesh_->addTile(tileData, static_cast<int>(tileHeader.tileDataSize), DT_TILE_FREE_DATA, 0, &resultRef);
            
            if (dtStatusFailed(status) || resultRef == 0) {
                printf("[NavError] Failed to add tile index %d. Status Flags: %u\n", i, status);
                dtFree(tileData, DT_ALLOC_PERM_TILE_DATA);
                Release();
                return false;
            }
        }
        
        int32_t activeTileCount = 0;
        for (int32_t i = 0; i < navMesh_->getMaxTiles(); ++i)
        {
            const dtMeshTile* tile = navMesh_->getTile(i);
            if (tile && tile->header)
            {
                activeTileCount++;
            }
        }
        // consoleLogger->Log(Color::Blue, L"[NavMesh Debug] 파일에서 로드된 타일 개수: %d | 서버 메모리에 실제 등록된 활성 타일 개수: %d\n", header.tileCount, activeTileCount);
        
#if WITH_NAVMESH_SEGMENT_LINKS
        // UE5 세그먼트 링크 재생성 후처리 패스
        // 모든 타일이 addTile로 등록된 상태에서 수행해야 사방의 이웃 타일을 정상적으로 참조합니다.
        for (int32_t i = 0; i < navMesh_->getMaxTiles(); ++i)
        {
            // 1. 인덱스로부터 타일 포인터를 가져옵니다 (getTile을 public으로 개방해야 함)
            const dtMeshTile* tile = navMesh_->getTile(i);
            if (tile == nullptr || tile->header == nullptr)
            {
                continue;
            }

            // 2. 타일 포인터로부터 고유 타일 참조 ID(dtTileRef)를 획득합니다.
            dtTileRef tileRef = navMesh_->getTileRef(tile);
            if (tileRef == 0)
            {
                continue;
            }

            // 3. UE 소스코드 방식 그대로 processSegmentLinksForTile를 호출합니다.
            unsigned int numSkippedNeighborTiles = 0;
            navMesh_->processSegmentLinksForTile(
                tileRef, 
                0,        // MaxSkippedNeigborTiles (UE 기본값 0)
                nullptr,  // OutSkippedNeigborTiles (UE 기본값 nullptr)
                numSkippedNeighborTiles
            );
        }
#endif
        
        // consoleLogger->Log(Color::Blue, L"[Navigation] Successfully loaded navigation mesh from file: %S\n", filePath.string().c_str());
        // consoleLogger->Log(Color::Blue, L"[Navigation] Loaded. tiles=%d, maxTiles=%d, maxPolys=%d\n", header.tileCount, header.maxTiles, header.maxPolys);
        
        // DebugPrintNavMeshBounds();
        
        return true;
    }

    bool ServerNavigation::FindNearestPoly(QueryContext& queryContext, const SE::Math::Vector3& pos, const SE::Math::Vector3& halfExtents,
        dtPolyRef& outRef, SE::Math::Vector3& outNearest) const
    {
        if (!IsLoaded() || !queryContext.IsValid())
            return false;
        
        dtReal p[3];
        dtReal e[3];
        dtReal nearest[3];
        
        ToDetour(pos, p);
        ToDetourExtents(halfExtents, e);
        
        outRef = 0;
        
        const dtStatus status = queryContext.navQuery_->findNearestPoly(p, e, queryContext.filter_, &outRef, nearest, nullptr);
        
        if (dtStatusFailed(status) || outRef == 0)
            return false;
        
        outNearest = FromDetour(nearest);
        return true;
    }

    NavPathResult  ServerNavigation::FindPath(QueryContext& queryContext, const SE::Math::Vector3& start, const SE::Math::Vector3& end,
        std::vector<SE::Math::Vector3>& outPath) const
    {
        using namespace SE::Math;
        
        outPath.clear();
        
        if (!IsLoaded() || !queryContext.IsValid())
            return NavPathResult::Failed;
        
        constexpr Vector3 startExtents{200.0f, 200.0f, 800.0f};
        constexpr Vector3 endExtents{500.0f, 500.0f, 800.0f};

        constexpr float maxEndProjectXY = 80.0f;
        
        dtPolyRef startRef = 0;
        dtPolyRef endRef = 0;
        
        Vector3 nearestStart{};
        Vector3 nearestEnd{};
        if (!FindNearestPoly(queryContext, start, startExtents, startRef, nearestStart))
        {
            return NavPathResult::StartNotOnNavMesh;
        }
        if (!FindNearestPoly(queryContext, end, endExtents, endRef, nearestEnd))
        {
            return NavPathResult::EndNotOnNavMesh;
        }
        
        const Vector3 endDiff = nearestEnd - end;
        const float endDiffXY = std::sqrt(endDiff.x * endDiff.x + endDiff.y * endDiff.y);

        if (endDiffXY > maxEndProjectXY)
        {
            // consoleLogger->Log(Color::Yellow,
            //     L"[FindPath] End projected too far. end=(%.1f %.1f %.1f) nearestEnd=(%.1f %.1f %.1f) diffXY=%.1f\n",
            //     end.x, end.y, end.z,
            //     nearestEnd.x, nearestEnd.y, nearestEnd.z,
            //     endDiffXY);

            return NavPathResult::EndNotOnNavMesh;
        }
        
        dtReal detourStart[3];
        dtReal detourEnd[3];
        
        ToDetour(nearestStart, detourStart);
        ToDetour(nearestEnd, detourEnd);
        
        dtQueryResult pathResult;
        pathResult.reserve(maxPathPolys_);

        constexpr dtReal costLimit = static_cast<dtReal>(FLT_MAX);

        dtStatus status = queryContext.navQuery_->findPath(
            startRef,
            endRef,
            detourStart,
            detourEnd,
            costLimit,
            queryContext.filter_,
            pathResult,
            nullptr);
        
        if (dtStatusFailed(status) || pathResult.size() <= 0)
            return NavPathResult::Failed;
        
        std::vector<dtPolyRef> polys(pathResult.size());
        pathResult.copyRefs(polys.data(), static_cast<int>(polys.size()));

        dtQueryResult straightResult;
        straightResult.reserve(maxStraightPath_);
        
        status = queryContext.navQuery_->findStraightPath(
            detourStart,
            detourEnd,
            polys.data(),
            static_cast<int>(polys.size()),
            straightResult,
            DT_STRAIGHTPATH_AREA_CROSSINGS);
        
        if (dtStatusFailed(status) || straightResult.size() <= 0)
            return NavPathResult::Failed;
        
        outPath.reserve(straightResult.size());

        for (int i = 0; i < straightResult.size(); ++i)
        {
            outPath.push_back(FromDetour(straightResult.getPos(i)));
        }
        
        return NavPathResult::Success;
    }

    bool ServerNavigation::IsReachablePoly(dtPolyRef startRef, dtPolyRef endRef) const
    {
        if (startRef == 0 || endRef == 0)
            return false;

        if (startRef == endRef)
            return true;
        
        return false;
    }

    bool ServerNavigation::IsReachablePosition(QueryContext& queryContext, const SE::Math::Vector3& start, const SE::Math::Vector3& end,
        const SE::Math::Vector3& halfExtents) const
    {
        if (!IsLoaded() || !queryContext.IsValid())
            return false;

        dtPolyRef startRef{};
        dtPolyRef endRef{};
        SE::Math::Vector3 startNearest{};
        SE::Math::Vector3 endNearest{};

        if (!FindNearestPoly(queryContext, start, halfExtents, startRef, startNearest) || startRef == 0)
            return false;

        if (!FindNearestPoly(queryContext, end, halfExtents, endRef, endNearest) || endRef == 0)
            return false;
        
        // 임시 구현: 현재는 FindPath 기반
        // 나중에 component ID 방식으로 교체
        std::vector<SE::Math::Vector3> path;
        NavPathResult result = FindPath(queryContext, startNearest, endNearest, path);
        
        return result == NavPathResult::Success;

        // return IsReachablePoly(startRef, endRef);
    }

    bool ServerNavigation::ProjectToNavMesh(QueryContext& queryContext, const Math::Vector3& pos, Math::Vector3& outPos) const
    {
        if (!IsLoaded() || !queryContext.IsValid())
            return false;

        constexpr SE::Math::Vector3 halfExtents{
            100.0f,   // x
            100.0f,   // y
            500.0f    // z → Detour 수직 탐색 범위
        };

        dtPolyRef nearestRef = 0;
        SE::Math::Vector3 nearestPos{};

        if (!FindNearestPoly(queryContext, pos, halfExtents, nearestRef, nearestPos))
            return false;

        outPos = nearestPos;
        return true;
    }

    bool ServerNavigation::MoveAlongSurface(QueryContext& queryContext, const Math::Vector3& start, const Math::Vector3& end,
        Math::Vector3& outPos) const
    {
        if (!IsLoaded() || !queryContext.IsValid())
            return false;

        constexpr Math::Vector3 halfExtents{100.0f, 100.0f, 500.0f};

        dtPolyRef startRef = 0;
        Math::Vector3 nearestStart{};

        if (!FindNearestPoly(queryContext, start, halfExtents, startRef, nearestStart))
            return false;

        dtPolyRef endRef = 0;
        Math::Vector3 nearestEnd{};
        if (!FindNearestPoly(queryContext, end, halfExtents, endRef, nearestEnd))
            return false;

        dtReal detourStart[3];
        dtReal detourEnd[3];
        dtReal result[3];

        ToDetour(nearestStart, detourStart);
        ToDetour(nearestEnd, detourEnd);    // end → nearestEnd로 교체

        dtPolyRef visited[16]{};
        int visitedCount = 0;

        const dtStatus status = queryContext.navQuery_->moveAlongSurface(
            startRef,
            detourStart,
            detourEnd,
            queryContext.filter_,
            result,
            visited,
            &visitedCount,
            16
        );

        if (dtStatusFailed(status))
            return false;

        outPos = FromDetour(result);

        dtReal height = 0.0f;
        if (dtStatusSucceed(queryContext.navQuery_->getPolyHeight(visitedCount > 0 ? visited[visitedCount - 1] : startRef, result, &height)))
            outPos.z = static_cast<float>(height);

        return true;
    }

    bool ServerNavigation::DebugValidatePoint(const SE::Math::Vector3& pos) const
    {
        if (!IsLoaded())
            return false;
        
        dtPolyRef ref = 0;
        SE::Math::Vector3 nearest;
        
        constexpr SE::Math::Vector3 halfExtents{100.0f, 100.0f, 300.0f};
        
        QueryContext queryContext;
        if (!queryContext.Init(*this)) {
            consoleLogger->Log(Color::Red, L"[Navigation] DebugValidatePoint query init failed.\n");
            return false;
        }

        if (!FindNearestPoly(queryContext, pos, halfExtents, ref, nearest)) {
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

        for (int i = 0; i < navMesh_->getMaxTiles(); ++i) {
            const dtMeshTile* tile = navMesh_->getTile(i);
            if (tile == nullptr || tile->header == nullptr)
                continue;
        
            const dtMeshHeader* h = tile->header;
        
            consoleLogger->Log(Color::Cyan,
                L"[NavMeshTile %d] bmin=(%.2f %.2f %.2f), bmax=(%.2f %.2f %.2f), polys=%d, verts=%d\n",
                i,
                (double)h->bmin[0],
                (double)h->bmin[1],
                (double)h->bmin[2],
                (double)h->bmax[0],
                (double)h->bmax[1],
                (double)h->bmax[2],
                h->polyCount,
                h->vertCount);
        }
    }

    void ServerNavigation::DebugFindTilesAround(const SE::Math::Vector3& serverPos) const
    {
        dtReal p[3];
        ToDetour(serverPos, p);

        consoleLogger->Log(Color::Cyan,
            L"[DebugFindTilesAround] server=(%.2f %.2f %.2f), detour=(%.2f %.2f %.2f)\n",
            serverPos.x, serverPos.y, serverPos.z,
            (double)p[0], (double)p[1], (double)p[2]);

        for (int i = 0; i < navMesh_->getMaxTiles(); ++i) {
            const dtMeshTile* tile = navMesh_->getTile(i);
            if (tile == nullptr || tile->header == nullptr)
                continue;
        
            const dtMeshHeader* h = tile->header;
        
            const bool nearX = p[0] >= h->bmin[0] - 1000.0 && p[0] <= h->bmax[0] + 1000.0;
            const bool nearZ = p[2] >= h->bmin[2] - 1000.0 && p[2] <= h->bmax[2] + 1000.0;
        
            if (nearX && nearZ) {
                consoleLogger->Log(Color::Yellow,
                    L"[NearTile %d] bmin=(%.2f %.2f %.2f), bmax=(%.2f %.2f %.2f), polys=%d\n",
                    i,
                    (double)h->bmin[0], (double)h->bmin[1], (double)h->bmin[2],
                    (double)h->bmax[0], (double)h->bmax[1], (double)h->bmax[2],
                    h->polyCount);
            }
        }
    }

    bool ServerNavigation::DebugExportObj(const std::filesystem::path& filePath) const
    {
        if (navMesh_ == nullptr)
            return false;

        std::ofstream out(filePath);
        if (!out.is_open())
            return false;

        out << "# Server NavMesh Debug OBJ\n";

        int vertexIndex = 1;

        for (int tileIndex = 0; tileIndex < navMesh_->getMaxTiles(); ++tileIndex)
        {
            const dtMeshTile* tile = navMesh_->getTile(tileIndex);
            if (tile == nullptr || tile->header == nullptr || tile->verts == nullptr || tile->polys == nullptr)
                continue;

            out << "\n# Tile " << tileIndex << "\n";

            for (int polyIndex = 0; polyIndex < tile->header->polyCount; ++polyIndex)
            {
                const dtPoly* poly = &tile->polys[polyIndex];

                // OffMeshConnection은 일반 NavMesh 표면이 아니므로 제외
                if (poly->vertCount < 3)
                    continue;

                const dtPolyDetail* detail = &tile->detailMeshes[polyIndex];

                for (int triIndex = 0; triIndex < detail->triCount; ++triIndex)
                {
                    const unsigned char* tri = &tile->detailTris[(detail->triBase + triIndex) * 4];

                    int face[3]{};

                    for (int j = 0; j < 3; ++j)
                    {
                        const dtReal* v = nullptr;

                        if (tri[j] < poly->vertCount)
                        {
                            v = &tile->verts[poly->verts[tri[j]] * 3];
                        }
                        else
                        {
                            const int detailVertIndex = detail->vertBase + (tri[j] - poly->vertCount);
                            v = &tile->detailVerts[detailVertIndex * 3];
                        }

                        const SE::Math::Vector3 serverPos = FromDetour(v);

                        out << "v "
                            << serverPos.x << " "
                            << -serverPos.y << " "
                            << serverPos.z << "\n";

                        face[j] = vertexIndex++;
                    }

                    out << "f " << face[2] << " " << face[1] << " " << face[0] << "\n";
                }
            }
        }

        return true;
    }

    void ServerNavigation::Release()
    {
        if (navMesh_) {
            dtFreeNavMesh(navMesh_);
            navMesh_ = nullptr;
        }
    }

    void ServerNavigation::ToDetour(const SE::Math::Vector3& in, dtReal out[3])
    {
        // UE RecastHelpers.cpp 기준
        // Unreal2RecastPoint = (-X, Z, -Y)
        out[0] = static_cast<dtReal>(-in.x);
        out[1] = static_cast<dtReal>( in.z);
        out[2] = static_cast<dtReal>(-in.y);
    }
    
    void ServerNavigation::ToDetourExtents(const SE::Math::Vector3& in, dtReal out[3])
    {
        // Extent는 부호가 의미 없으므로 abs 축 변환
        out[0] = static_cast<dtReal>(std::abs(in.x));
        out[1] = static_cast<dtReal>(std::abs(in.z));
        out[2] = static_cast<dtReal>(std::abs(in.y));
    }

    SE::Math::Vector3 ServerNavigation::FromDetour(const dtReal in[3])
    {
        // Recast2UnrealPoint = (-X, -Z, Y)
        return SE::Math::Vector3(
            static_cast<float>(-in[0]),
            static_cast<float>(-in[2]),
            static_cast<float>( in[1]));
    }
}

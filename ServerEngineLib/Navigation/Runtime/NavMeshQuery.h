#pragma once
#include <memory>
#include <vector>
#include "Navigation/NavMeshTypes.h"
#include "Navigation/NavMeshCommon.h"

class dtQueryFilter;
class dtNavMesh;

/*----------------
   NavMeshQuery
----------------*/
//
// NavMeshQuery
// 

namespace SE::Navigation
{
    class NavMeshQuery
    {
    public:
        NavMeshQuery();
        ~NavMeshQuery();
        
        bool Initialize(dtNavMesh* navMesh, int maxNodes = 2048);
        
        bool FindNearestPoint(const Vector3& pos, const Vector3& halfExtents, Vector3& outPoint, PolyRef& outPolyRef) const;
        
        PathResult FindPath(const Vector3& start, const Vector3& end, const Vector3& halfExtents) const;
        
    private:
        std::unique_ptr<dtNavMeshQuery, DtNavMeshQueryDeleter> query_;
        std::unique_ptr<dtQueryFilter> filter_;
    
    };
}


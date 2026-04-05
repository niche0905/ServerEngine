#pragma once
#include <memory>

class dtNavMesh;
class dtNavMeshQuery;
class dtQueryFilter;

namespace SE::Navigation
{
    struct DtNavMeshDeleter
    {
        void operator()(dtNavMesh* ptr) const;
    };
    
    struct DtNavMeshQueryDeleter
    {
        void operator()(dtNavMeshQuery* ptr) const;
    };
    
}

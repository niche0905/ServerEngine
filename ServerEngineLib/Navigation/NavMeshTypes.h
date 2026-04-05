#pragma once
#include <vector>
#include <string>

namespace SE::Navigation
{
    using namespace SE::Math;
    
    struct PolyRef
    {
        uint64 value = 0;
    };
    
    struct PathPoint
    {
        Vector3 position;
    };
    
    struct PathResult
    {
        bool success = false;
        bool partial = false;
        std::vector<PathPoint> points;
    };
    
}

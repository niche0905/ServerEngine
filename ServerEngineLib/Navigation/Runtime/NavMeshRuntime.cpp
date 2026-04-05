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
    bool NavMeshRuntime::LoadFromFile(const std::filesystem::path& filePath)
    {
        NavMeshFileData loaded{};
        if (!NavMeshFileReader::ReadFromFile(filePath, loaded))
            return false;
        
        auto navMesh = NavMeshLoader::CreateNavMeshFromFile(loaded);
        if (!navMesh)
            return false;
        
        fileData_ = std::move(loaded);
        navMesh_ = std::move(navMesh);
        
        return true;
    }
}

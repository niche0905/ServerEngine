#pragma once
#include <filesystem>
#include <memory>
#include "Navigation/NavMeshCommon.h"
#include "Navigation/File/NavMeshFileFormat.h"

class dtNavMesh;

/*------------------
   NavMeshRuntime
------------------*/
//
// NavMeshRuntime는 경로를 받아 dtNavMesh 객체를 사용하여 실제 경로 탐색을 수행하는 런타임 클래스입니다.
// NavMeshRuntime은 NavMeshFileReader를 사용하여 NavMeshFileData를 채우고, 
// NavMeshLoader를 사용하여 dtNavMesh 객체를 생성합니다.
// 

namespace SE::Navigation
{
    class NavMeshRuntime
    {
    public:
        bool LoadFromFile(const std::filesystem::path& filePath);
        
        dtNavMesh* GetNavMesh() const { return navMesh_.get(); }
        const NavMeshFileData& GetFileData() const { return fileData_; }
        
        bool IsLoaded() const { return navMesh_ != nullptr; }
    
    private:
        NavMeshFileData fileData_{};
        std::unique_ptr<dtNavMesh, DtNavMeshDeleter> navMesh_;
        
    };
}


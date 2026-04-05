#pragma once
#include <filesystem>
#include <memory>
#include "Navigation/File/NavMeshFileFormat.h"
#include "Navigation/NavMeshCommon.h"

class dtNavMesh;

/*-----------------
   NavMeshLoader
-----------------*/
//
// NavMeshLoader는 런타임 중에 파싱한 네비게이션 메시 데이터를 기반으로 dtNavMesh 객체를 생성하는 역할을 합니다.
// recastnavigation 라이브러리의 dtNavMesh 객체는 네비게이션 메시의 핵심 데이터 구조로, 경로 탐색과 관련된 다양한 기능을 제공합니다.
// 

namespace SE::Navigation
{
   class NavMeshLoader
   {
   public:
      static std::unique_ptr<dtNavMesh, DtNavMeshDeleter> CreateNavMeshFromFile(const NavMeshFileData& fileData);
    
   };
   
}


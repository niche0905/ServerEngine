#pragma once
#include <filesystem>
#include "Navigation/Runtime/NavMeshRuntime.h"
#include "Navigation/Runtime/NavMeshQuery.h"

/*---------------------
   NavigationService
---------------------*/
//
// NavigationService는 네비게이션 메시를 로드하고, 네비게이션 관련 쿼리를 처리하는 서비스입니다.
// 

namespace SE::Navigation
{
   class NavigationService
   {
   public:
      bool Initialize(const std::filesystem::path& navFilePath);
      
      bool ProjectPoint(const Vector3& pos, Vector3& outProjected) const;
      PathResult FindPath(const Vector3& start, const Vector3& end) const;
      
   private:
      NavMeshRuntime runtime_;
      mutable NavMeshQuery query_;
      Vector3 defaultHalfExtent_{2.0f, 4.0f, 2.0f};
    
   };
}


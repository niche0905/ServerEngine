#include "pch.h"
#include "NavigationService.h"

/*---------------------
   NavigationService
---------------------*/

namespace SE::Navigation
{
   bool NavigationService::Initialize(const std::filesystem::path& navFilePath)
   {
      if (!runtime_.LoadFromFile(navFilePath))
         return false;
      
      if (!query_.Initialize(runtime_.GetNavMesh()))
         return false;
      
      return true;
   }

   bool NavigationService::ProjectPoint(const Vector3& pos, Vector3& outProjected) const
   {
      PolyRef ref{};
      return query_.FindNearestPoint(pos, defaultHalfExtent_, outProjected, ref);
   }

   PathResult NavigationService::FindPath(const Vector3& start, const Vector3& end) const
   {
      return query_.FindPath(start, end, defaultHalfExtent_);
   }
}

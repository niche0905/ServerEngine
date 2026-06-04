#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "ThirdParty/UEDetour/DetourNavMesh.h"
#include "Utils/Types.h"

class dtNavMesh;
class dtNavMeshQuery;
class dtQueryFilter;

enum class NavPathResult
{
   Success,
   StartNotOnNavMesh,
   EndNotOnNavMesh,
   Failed
};

/*---------------------
   ServerNavigation
---------------------*/
//
// ServerNavigation는 UE의 Detour코드를 이용하여 네비게이션 메시를 로드하고, 길찾기 기능을 제공하는 클래스입니다.
//

namespace SE::Nav
{
   class ServerNavigation
   {
   public:
      class QueryContext
      {
         friend class ServerNavigation;

      public:
         QueryContext() = default;
         ~QueryContext();

         QueryContext(const QueryContext&) = delete;
         QueryContext& operator=(const QueryContext&) = delete;

         bool Init(const ServerNavigation& navigation);
         void Release();
         bool IsValid() const { return navQuery_ != nullptr && filter_ != nullptr; }

      private:
         dtNavMeshQuery*               navQuery_ = nullptr;
         dtQueryFilter*                filter_ = nullptr;
      };

   public:
      ServerNavigation() = default;
      ~ServerNavigation();
      
      ServerNavigation(const ServerNavigation&) = delete;
      ServerNavigation& operator=(const ServerNavigation&) = delete;
      
      bool LoadFromFile(const std::filesystem::path& filePath);
      
      bool FindNearestPoly(QueryContext& queryContext, const SE::Math::Vector3& pos, const SE::Math::Vector3& halfExtents, dtPolyRef& outRef, SE::Math::Vector3& outNearest) const;
    
      NavPathResult FindPath(QueryContext& queryContext, const SE::Math::Vector3& start, const SE::Math::Vector3& end, std::vector<SE::Math::Vector3>& outPath) const;
      
      bool IsReachablePoly(dtPolyRef startRef, dtPolyRef endRef) const;
      bool IsReachablePosition(QueryContext& queryContext, const SE::Math::Vector3& start, const SE::Math::Vector3& end, const SE::Math::Vector3& halfExtents) const;
      
      bool ProjectToNavMesh(QueryContext& queryContext, const Math::Vector3& pos, Math::Vector3& outPos) const;
      
      bool MoveAlongSurface(QueryContext& queryContext, const Math::Vector3& start, const Math::Vector3& end, Math::Vector3& outPos) const;
      
      bool IsLoaded() const { return navMesh_ != nullptr; }
      
   public:
      bool DebugValidatePoint(const SE::Math::Vector3& pos) const;
      void DebugPrintNavMeshBounds() const;
      void DebugFindTilesAround(const SE::Math::Vector3& serverPos) const;
      bool DebugExportObj(const std::filesystem::path& filePath) const;
      
   private:
      void Release();
      
      static void ToDetour(const SE::Math::Vector3& in, dtReal out[3]);
      static void ToDetourExtents(const SE::Math::Vector3& in, dtReal out[3]);
      static SE::Math::Vector3 FromDetour(const dtReal in[3]);
      
   private:
      dtNavMesh*                    navMesh_ = nullptr;
      
      // int32                         maxSearchNodes_ = 4096;
      // int32                         maxPathPolys_ = 256;
      // int32                         maxStraightPath_ = 256;
      int32                         maxSearchNodes_ = 16384;
      int32                         maxPathPolys_ = 1024;
      int32                         maxStraightPath_ = 512;
      
   };

}

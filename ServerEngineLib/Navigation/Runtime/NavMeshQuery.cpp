#include "pch.h"
#include "NavMeshQuery.h"
#include "Navigation/Runtime/NavMeshQuery.h"
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/DetourNavMeshQuery.h>
#include <recastnavigation/DetourAlloc.h>

/*----------------
   NavMeshQuery
----------------*/

namespace SE::Navigation
{
   void DtNavMeshQueryDeleter::operator()(dtNavMeshQuery* ptr) const
   {
      if (ptr) {
         dtFreeNavMeshQuery(ptr);
      }
   }
   
   namespace
   {
      inline void ToFloat3(const Vector3& v, float out[3])
      {
         out[0] = v.x;
         out[1] = v.y;
         out[2] = v.z;
      }
      
      inline Vector3 FromFloat3(const float v[3])
      {
         return Vector3{v[0], v[1], v[2]};
      }
   }
   
   NavMeshQuery::NavMeshQuery() = default;
   NavMeshQuery::~NavMeshQuery() = default;

   bool NavMeshQuery::Initialize(dtNavMesh* navMesh, int maxNodes)
   {
      if (!navMesh) {
         return false;   // 유효하지 않은 네비게이션 메시
      }
      
      dtNavMeshQuery* rawQuery = dtAllocNavMeshQuery();
      if (!rawQuery) {
         return false;   // 메모리 할당 실패
      }
      
      if (dtStatusFailed(rawQuery->init(navMesh, maxNodes))) {
         dtFreeNavMeshQuery(rawQuery);
         return false;   // 네비게이션 메시 쿼리 초기화 실패
      }
      
      query_.reset(rawQuery);
      filter_ = std::make_unique<dtQueryFilter>();
      
      return true;
      
   }

   bool NavMeshQuery::FindNearestPoint(const Vector3& pos, const Vector3& halfExtents, Vector3& outPoint,
      PolyRef& outPolyRef) const
   {
      if (!query_ or !filter_)
         return false;
      
      float p[3], ext[3], nearest[3];
      ToFloat3(pos, p);
      ToFloat3(halfExtents, ext);
      
      dtPolyRef ref = 0;
      const dtStatus status = query_->findNearestPoly(p, ext, filter_.get(), &ref, nearest);
      
      if (dtStatusFailed(status) or ref == 0) {
         return false;   // 가장 가까운 폴리곤을 찾지 못함
      }
      
      outPolyRef.value = static_cast<uint64>(ref);
      outPoint = FromFloat3(nearest);
      
      return true;
   }

   PathResult NavMeshQuery::FindPath(const Vector3& start, const Vector3& end, const Vector3& halfExtents) const
   {
      PathResult result{};
      
      if (!query_ or !filter_)
         return result;
      
      float s[3], e[3], ext[3];
      ToFloat3(start, s);
      ToFloat3(end, e);
      ToFloat3(halfExtents, ext);
      
      dtPolyRef startRef = 0;
      dtPolyRef endRef = 0;
      float startNearest[3]{};
      float endNearest[3]{};
      
      if (dtStatusFailed(query_->findNearestPoly(s, ext, filter_.get(), &startRef, startNearest)) or startRef == 0) {
         return result;   // 시작점에 가장 가까운 폴리곤을 찾지 못함
      }
      if (dtStatusFailed(query_->findNearestPoly(e, ext, filter_.get(), &endRef, endNearest)) or endRef == 0) {
         return result;   // 도착점에 가장 가까운 폴리곤을 찾지 못함
      }
      
      constexpr int kMaxPolys = 256;
      dtPolyRef polys[kMaxPolys]{};
      int polyCount = 0;
      
      if (dtStatusFailed(query_->findPath(startRef, endRef, startNearest, endNearest, filter_.get(), polys, &polyCount, kMaxPolys))) {
         return result;
      }
      
      if (polyCount <= 0) {
         return result;   // 경로를 찾지 못함
      }
      
      constexpr int kMaxStraight = 256;
      float straightPts[kMaxStraight * 3]{};
      unsigned char straightFlags[kMaxStraight]{};
      dtPolyRef straightRefs[kMaxStraight]{};
      int straightCount = 0;
      
      const dtStatus straightStatus = query_->findStraightPath(startNearest, endNearest, polys, polyCount, straightPts, straightFlags, straightRefs, &straightCount, kMaxStraight);
      
      if (dtStatusFailed(straightStatus) or straightCount <= 0) {
         return result;   // 직선 경로를 찾지 못함
      }
      
      result.success = true;
      result.partial = (polys[polyCount - 1] != endRef);
      
      result.points.reserve(straightCount);
      for (int i = 0; i < straightCount; ++i) {
         result.points.push_back(PathPoint{Vector3{straightPts[i * 3 + 0], straightPts[i * 3 + 1], straightPts[i * 3 + 2] }});
      }
      
      return result;
   }
}

#pragma once
#include "Data/Navigation/ServerNavigation.h"
#include "Physics/Collider/Collider.h"
#include "Spatial/UniformGridSpatial.h"

namespace SE::Physics
{
   class Collider;
   class AABBCollider;
}

namespace se::map
{
   struct LoadedMapData;
}

/*-------------
   ServerMap
-------------*/
//
// ServerMap는 Spatial을 포함하는 정적 맵 데이터를 나타냅니다.
//

class ServerMap
{
public:
   bool BuildFromLoadedData(const se::map::LoadedMapData& loadedData);
   bool LoadNavigation(const std::filesystem::path& navMeshPath);
   
public:
   const SE::Physics::Collider* GetCollider(uint32 colliderId) const;
   void QueryAABB(const SE::Physics::AABBCollider& query, std::vector<uint32>& outColliderIds) const;
   
   bool Raycast(const SE::Physics::Ray& ray, SE::Physics::RaycastHit& outResult) const;
   bool Intersect(const SE::Physics::Collider& other, SE::Physics::CollisionResult& outResult) const;
   bool SphereCast(const SE::Math::Vector3& from, const SE::Math::Vector3& to, float radius, SE::Physics::RaycastHit& outResult) const;
   
public:
   const SE::Nav::ServerNavigation& GetNavigation() const { return navigation_; }
   bool HasNavigation() const { return navigation_.IsLoaded(); }
   
   bool FindNearestPoly(const SE::Math::Vector3& pos, const SE::Math::Vector3& halfExtents, dtPolyRef& outRef, SE::Math::Vector3& outNearest) const;
   NavPathResult  FindPath(const SE::Math::Vector3& start, const SE::Math::Vector3& end, std::vector<SE::Math::Vector3>& outPath) const;
   bool ProjectToNavMesh(const SE::Math::Vector3& pos, SE::Math::Vector3& outPos) const;
   bool MoveAlongSurface(const SE::Math::Vector3& start, const SE::Math::Vector3& end, SE::Math::Vector3& outPos) const;
   
private:
   std::vector<std::unique_ptr<SE::Physics::Collider>>         colliders_;
   UniformGridSpatial                                          spatial_{};
   
   SE::Nav::ServerNavigation                                   navigation_;
    
};

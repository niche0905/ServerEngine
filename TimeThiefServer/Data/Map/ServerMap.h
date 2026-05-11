#pragma once
#include "Physics/Collider/Collider.h"

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
   
   const SE::Physics::Collider* GetCollider(uint32 colliderId) const;
   void QueryAABB(const SE::Physics::AABBCollider& query, std::vector<uint32>& outColliderIds) const;
   
private:
   std::vector<std::unique_ptr<SE::Physics::Collider>> colliders_;
   // UniformGridSpatial
    
};

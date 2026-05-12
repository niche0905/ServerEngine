#pragma once
#include "Utils/Types.h"
#include "Physics/Collider/Collider.h"

struct GridCell
{
   std::vector<uint32> colliderIds; // 이 셀에 포함된 콜라이더 ID 목록
};

/*----------------------
   UniformGridSpatial
----------------------*/
//
// UniformGridSpatial는 맵 공간을 균등한 격자로 나누어 객체들을 관리하는 공간 분할 시스템입니다.
//

class UniformGridSpatial
{
public:
   void Build(const std::vector<std::unique_ptr<SE::Physics::Collider>>& colliders, float cellSize);
   void QueryAABB(const SE::Physics::AABBCollider& query, std::vector<uint32>& outColliderIds) const;
   void QueryRay(const SE::Physics::Ray& ray, std::vector<uint32>& outColliderIds) const;
   
private:
   Int2 WorldToCell(const SE::Math::Vector3& position) const;
   
private:
   float cellSize_ = 10000.0f;   // 각 격자 셀의 크기 (예: 100m x 100m)
   SE::Math::Vector3 origin_{};
   
   int32 width_ = 0;
   int32 height_ = 0;
   
   std::vector<GridCell> cells_;
    
};

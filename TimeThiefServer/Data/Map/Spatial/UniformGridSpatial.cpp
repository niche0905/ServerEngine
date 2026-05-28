#include "pch.h"
#include "UniformGridSpatial.h"
#include "Physics/Collider/AABBCollider.h"
#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <limits>
#include "Physics/Ray/Ray.h"

/*----------------------
   UniformGridSpatial
----------------------*/

void UniformGridSpatial::Build(const std::vector<std::unique_ptr<SE::Physics::Collider>>& colliders, float cellSize)
{
   cellSize_ = cellSize;
   cells_.clear();
   
   if (colliders.empty() or cellSize <= 0.0f) {
      consoleLogger->Log(Color::Red, L"[Spatial] Build called with invalid parameters. Collider count: %zu, CellSize: %f\n", colliders.size(), cellSize);
      origin_ = {};
      width_ = 0;
      height_ = 0;
      return;   // 유효하지 않은 입력 처리
   }
   
   SE::Math::Vector3 worldMin{
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max()
  };

   SE::Math::Vector3 worldMax{
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest(),
      std::numeric_limits<float>::lowest()
  };
   
   // 1. 전체 맵 AABB 계산
   for (const auto& collider : colliders)
   {
      if (!collider)
         continue;

      const auto& aabb = collider->GetWorldAABB();

      const auto min = aabb.GetMin();
      const auto max = aabb.GetMax();

      worldMin.x = std::min(worldMin.x, min.x);
      worldMin.y = std::min(worldMin.y, min.y);
      worldMin.z = std::min(worldMin.z, min.z);

      worldMax.x = std::max(worldMax.x, max.x);
      worldMax.y = std::max(worldMax.y, max.y);
      worldMax.z = std::max(worldMax.z, max.z);
   }
   
   origin_ = worldMin;
   
   width_ = static_cast<int32>(std::ceil((worldMax.x - worldMin.x) / cellSize_)) + 1;
   height_ = static_cast<int32>(std::ceil((worldMax.y - worldMin.y) / cellSize_)) + 1;
   
   cells_.resize(static_cast<size_t>(width_ * height_));
   
   // 2. 각 Collider를 겹치는 Cell에 등록
   for (uint32 colliderId = 0; colliderId < colliders.size(); ++colliderId)
   {
      const auto& collider = colliders[colliderId];
      if (!collider)
         continue;

      const auto& aabb = collider->GetWorldAABB();

      SE::Math::Vector3 min = aabb.GetMin();
      SE::Math::Vector3 max = aabb.GetMax();

      Int2 minCell = WorldToCell(min);
      Int2 maxCell = WorldToCell(max);

      minCell.x = std::clamp(minCell.x, 0, width_ - 1);
      minCell.y = std::clamp(minCell.y, 0, height_ - 1);
      maxCell.x = std::clamp(maxCell.x, 0, width_ - 1);
      maxCell.y = std::clamp(maxCell.y, 0, height_ - 1);

      for (int32 y = minCell.y; y <= maxCell.y; ++y) {
         for (int32 x = minCell.x; x <= maxCell.x; ++x) {
            
            const int32 index = y * width_ + x;
            cells_[index].colliderIds.push_back(colliderId);
         }
      }
   }
   
   // Test Log
   // {
   //    consoleLogger->Log(Color::Blue, L"[Spatial] Spatial Grid Build Success, width: %u height: %u\n", width_, height_);
   //    
   //    for (int32 y = 0; y < height_; ++y) {
   //       for (int32 x = 0; x < width_; ++x) {
   //          const int32 index = y * width_ + x;
   //          const GridCell& cell = cells_[index];
   //          consoleLogger->Log(Color::Blue, L"Cell (%d, %d) - Collider Count: %zu\n", x, y, cell.colliderIds.size());
   //       }
   //    }
   // }
}

void UniformGridSpatial::QueryPoint(const SE::Math::Vector3& point, std::vector<uint32>& outColliderIds) const
{
   outColliderIds.clear();

   if (cells_.empty() || width_ <= 0 || height_ <= 0)
      return;

   Int2 cell = WorldToCell(point);

   if (cell.x < 0 || cell.x >= width_ ||
       cell.y < 0 || cell.y >= height_)
   {
      return;
   }

   const int32 index = cell.y * width_ + cell.x;
   const GridCell& gridCell = cells_[index];

   outColliderIds.reserve(outColliderIds.size() + gridCell.colliderIds.size());

   for (uint32 colliderId : gridCell.colliderIds) {
      outColliderIds.push_back(colliderId);
   }
}

void UniformGridSpatial::QueryAABB(const SE::Physics::AABBCollider& query, std::vector<uint32>& outColliderIds) const
{
   outColliderIds.clear();

   if (cells_.empty() || width_ <= 0 || height_ <= 0)
      return;

   SE::Math::Vector3 min = query.GetMin();
   SE::Math::Vector3 max = query.GetMax();

   Int2 minCell = WorldToCell(min);
   Int2 maxCell = WorldToCell(max);

   minCell.x = std::clamp(minCell.x, 0, width_ - 1);
   minCell.y = std::clamp(minCell.y, 0, height_ - 1);
   maxCell.x = std::clamp(maxCell.x, 0, width_ - 1);
   maxCell.y = std::clamp(maxCell.y, 0, height_ - 1);

   std::unordered_set<uint32> uniqueIds;

   for (int32 y = minCell.y; y <= maxCell.y; ++y) {
      for (int32 x = minCell.x; x <= maxCell.x; ++x) {
         
         const int32 index = y * width_ + x;
         const GridCell& cell = cells_[index];

         for (uint32 colliderId : cell.colliderIds) {
            if (uniqueIds.insert(colliderId).second) {
               outColliderIds.push_back(colliderId);
            }
         }
      }
   }
}

void UniformGridSpatial::QueryRay(const SE::Physics::Ray& ray, std::vector<uint32>& outColliderIds) const
{
   outColliderIds.clear();
   
   if (width_ <= 0 or height_ <= 0 or cellSize_ <= 0.0f)
      return;
   
   const auto& origin = ray.origin;
   const auto& dir = ray.direction;
   
   constexpr float EPSILON = 1e-6f;
   
   if (std::abs(dir.x) < EPSILON and std::abs(dir.y) < EPSILON) {
      // Ray가 거의 수직인 경우, 단일 셀만 검사
      Int2 cell = WorldToCell(origin);
      if (cell.x >= 0 && cell.x < width_ && cell.y >= 0 && cell.y < height_) {
         const int32 index = cell.y * width_ + cell.x;
         const GridCell& gridCell = cells_[index];
         outColliderIds.insert(outColliderIds.end(), gridCell.colliderIds.begin(), gridCell.colliderIds.end());
      }
      return;
   }
   
   Int2 cell = WorldToCell(origin);
   
   // TODO: 만약 Ray의 시작점이 그리드 범위를 벗어난 경우, 교차점 계산하여 시작 셀을 결정하는 로직 추가 가능 (현재는 단순히 반환)
   if (cell.x < 0 or cell.x >= width_ or cell.y < 0 or cell.y >= height_) {
      // Ray의 시작점이 그리드 범위를 벗어난 경우, 교차점 계산 필요 (생략 가능)
      return;
   }
   
   int32 stepX = 0;
   int32 stepY = 0;
   
   float tMaxX = std::numeric_limits<float>::infinity();
   float tMaxY = std::numeric_limits<float>::infinity();
   
   float tDeltaX = std::numeric_limits<float>::infinity();
   float tDeltaY = std::numeric_limits<float>::infinity();
   
   if (std::abs(dir.x) >= EPSILON) {
      stepX = (dir.x > 0.0f) ? 1 : -1;
      
      const float nextBoundaryX = (stepX > 0) ? static_cast<float>(cell.x + 1) * cellSize_ : static_cast<float>(cell.x) * cellSize_;
      
      tMaxX = (nextBoundaryX - origin.x) / dir.x;
      tDeltaX = cellSize_ / std::abs(dir.x);
   }
   
   if (std::abs(dir.y) >= EPSILON) {
      stepY = (dir.y > 0.0f) ? 1 : -1;
      
      const float nextBoundaryY = (stepY > 0) ? static_cast<float>(cell.y + 1) * cellSize_ : static_cast<float>(cell.y) * cellSize_;
      
      tMaxY = (nextBoundaryY - origin.y) / dir.y;
      tDeltaY = cellSize_ / std::abs(dir.y);
   }
   
   std::unordered_set<uint32> visitedColliderIds;
   visitedColliderIds.reserve(128);

   while (cell.x >= 0 and cell.x < width_ and cell.y >= 0 and cell.y < height_) {
      
      const int32 index = cell.y * width_ + cell.x;
      
      const GridCell& gridCell = cells_[index];
      
      for (uint32 colliderId : gridCell.colliderIds) {
         if (visitedColliderIds.insert(colliderId).second) {
            outColliderIds.push_back(colliderId);
         }
      }
      
      if (tMaxX < tMaxY) {
         cell.x += stepX;
         tMaxX += tDeltaX;
      }
      else {
         cell.y += stepY;
         tMaxY += tDeltaY;
      }
   }
}

Int2 UniformGridSpatial::WorldToCell(const SE::Math::Vector3& position) const
{
   const int32 cellX = static_cast<int32>(
       std::floor((position.x - origin_.x) / cellSize_)
   );

   const int32 cellY = static_cast<int32>(
       std::floor((position.y - origin_.y) / cellSize_)
   );

   return Int2{ cellX, cellY };
}

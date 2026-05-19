#include "pch.h"
#include "ServerMap.h"
#include "Data/Loader/ServerMapLoader.h"
#include "Physics/Collider/Collider.h"
#include "Physics/Collider/CapsuleCollider.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/OBBCollider.h"
#include "Physics/Collider/SphereCollider.h"
#include "Physics/Ray/RaycastHit.h"

namespace 
{
   constexpr float DegToRad(float deg)
   {
      return deg * (3.14159265358979323846f / 180.0f);
   }
   
   SE::Math::Vector3 Normalize(const SE::Math::Vector3& v)
   {
      float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
      if (length > 0.0f) {
         return SE::Math::Vector3{ v.x / length, v.y / length, v.z / length };
      }
      return SE::Math::Vector3{ 0.0f, 0.0f, 0.0f }; // 길이가 0인 경우는 예외 처리
   }
   
   struct BasisAxes
   {
      SE::Math::Vector3 axisX;
      SE::Math::Vector3 axisY;
      SE::Math::Vector3 axisZ;
   };
   
   BasisAxes MakeBasisFromEulerDeg(const SE::Math::Vector3& rotationDeg)
   {
      const float pitch = DegToRad(rotationDeg.x);
      const float yaw   = DegToRad(rotationDeg.y);
      const float roll  = DegToRad(rotationDeg.z);

      const float cp = std::cos(pitch);
      const float sp = std::sin(pitch);
      const float cy = std::cos(yaw);
      const float sy = std::sin(yaw);
      const float cr = std::cos(roll);
      const float sr = std::sin(roll);

      // Unreal 기준에 가까운 방식:
      // X = Forward, Y = Right, Z = Up
      // RotationDeg = Pitch(X), Yaw(Y), Roll(Z) 라고 가정
      SE::Math::Vector3 axisX
      {
         cp * cy,
         cp * sy,
         sp
      };

      SE::Math::Vector3 axisY
      {
         sr * sp * cy - cr * sy,
         sr * sp * sy + cr * cy,
         -sr * cp
      };

      SE::Math::Vector3 axisZ
      {
         -(cr * sp * cy + sr * sy),
         -(cr * sp * sy - sr * cy),
         cr * cp
      };

      return {
         Normalize(axisX),
         Normalize(axisY),
         Normalize(axisZ)
      };
   }
   
   std::unique_ptr<SE::Physics::Collider> MakeOBB(const se::map::ColliderData& data)
   {
      SE::Math::Vector3 center{ data.position.x, data.position.y, data.position.z };
      SE::Math::Vector3 extents{ data.extents.x, data.extents.y, data.extents.z };
      SE::Math::Vector3 rotationDeg{ data.rotationDeg.x, data.rotationDeg.y, data.rotationDeg.z };
      BasisAxes basis = MakeBasisFromEulerDeg(rotationDeg);
      
      return std::make_unique<SE::Physics::OBBCollider>(center, extents, basis.axisX, basis.axisY, basis.axisZ);
   }

   std::unique_ptr<SE::Physics::Collider> MakeSphere(const se::map::ColliderData& data)
   {
      SE::Math::Vector3 center{ data.position.x, data.position.y, data.position.z };
      
      return std::make_unique<SE::Physics::SphereCollider>(center, data.radius);
   }

   std::unique_ptr<SE::Physics::Collider> MakeCapsule(const se::map::ColliderData& data)
   {
      SE::Math::Vector3 center{ data.position.x, data.position.y, data.position.z };
      SE::Math::Vector3 rotationDeg{ data.rotationDeg.x, data.rotationDeg.y, data.rotationDeg.z };
      BasisAxes basis = MakeBasisFromEulerDeg(rotationDeg);
      const SE::Math::Vector3 capsuleAxis = basis.axisZ;
      const float halfSegmentLength = std::max(0.0f, data.halfHeight - data.radius);
      
      SE::Math::Vector3 pointA{
         center.x - capsuleAxis.x * halfSegmentLength, 
         center.y - capsuleAxis.y * halfSegmentLength,
         center.z - capsuleAxis.z * halfSegmentLength
      };

      SE::Math::Vector3 pointB{
         center.x + capsuleAxis.x * halfSegmentLength,
         center.y + capsuleAxis.y * halfSegmentLength,
         center.z + capsuleAxis.z * halfSegmentLength
      };
      
      return std::make_unique<SE::Physics::CapsuleCollider>(pointA, pointB, data.radius);
   }

   std::unique_ptr<SE::Physics::Collider> CreateColliderFromData(const se::map::ColliderData& data)
   {
      std::unique_ptr<SE::Physics::Collider> collider;
      
      switch (data.type)
      {
      case se::map::ColliderType::OBB:
         collider = MakeOBB(data);
         break;
         
      case se::map::ColliderType::Sphere:
         collider = MakeSphere(data);
         break;
         
      case se::map::ColliderType::Capsule:
         collider = MakeCapsule(data);
         break;
         
      default:
         consoleLogger->Log(Color::Red, L"[ServerMap] Unknown collider type in loaded data: %d\n", static_cast<int>(data.type));
         return nullptr;
      }
      
      if (!collider) {
         consoleLogger->Log(Color::Red, L"[ServerMap] Failed to create collider for type: %d\n", static_cast<int>(data.type));
         return nullptr;
      }
      
      return collider;
   }
}

/*-------------
   ServerMap
-------------*/

bool ServerMap::BuildFromLoadedData(const se::map::LoadedMapData& loadedData)
{
   colliders_.clear();
   
   colliders_.reserve(loadedData.colliders.size());
   
   for (const auto& colliderData : loadedData.colliders) {
      
      std::unique_ptr<SE::Physics::Collider> collider = CreateColliderFromData(colliderData);
      if (!collider) {
         consoleLogger->Log(Color::Red, L"[ServerMap] Failed to create collider from loaded data.\n");
         return false;
      }
      colliders_.push_back(std::move(collider));
   }
   
   // consoleLogger->Log(Color::Blue, L"[ServerMap] Successfully built ServerMap from loaded data. Collider count: %zu\n", colliders_.size());
   
   spatial_.Build(colliders_, 10000.0f);
   
   return true;
}

bool ServerMap::LoadNavigation(const std::filesystem::path& navMeshPath)
{
   // if (!navigation_.LoadFromFile(navMeshPath))
   //    return false;
   //
   // navigation_.DebugPrintNavMeshBounds();
   // if (navigation_.DebugExportObj("debug_server_navmesh.obj"))
   //    consoleLogger->Log(Color::Blue, L"[ServerMap] NavMesh Loaded successfully.\n");
   //
   // return true;
   return navigation_.LoadFromFile(navMeshPath);
}

const SE::Physics::Collider* ServerMap::GetCollider(uint32 colliderId) const
{
   if (colliderId >= colliders_.size()) {
      consoleLogger->Log(Color::Red, L"[ServerMap] GetCollider: Invalid colliderId %u (out of range)\n", colliderId);
      return nullptr;
   }
   
   return colliders_[colliderId].get();
}

void ServerMap::QueryAABB(const SE::Physics::AABBCollider& query, std::vector<uint32>& outColliderIds) const
{
   spatial_.QueryAABB(query, outColliderIds);
}

bool ServerMap::Raycast(const SE::Physics::Ray& ray, SE::Physics::RaycastHit& outResult) const
{
   std::vector<uint32> candidateIds;
   candidateIds.reserve(64);
   
   spatial_.QueryRay(ray, candidateIds);
   
   bool hit = false;
   float closestDistance = std::numeric_limits<float>::max();

   SE::Physics::RaycastHit tempHit{};
   
   for (uint32 colliderId : candidateIds) {
      const SE::Physics::Collider* colliderPtr = GetCollider(colliderId);
      if (!colliderPtr)
         continue;

      tempHit = {};
      
      if (!colliderPtr->Raycast(ray, tempHit))
         continue;
      
      if (tempHit.t >= closestDistance)
         continue;
      
      closestDistance = tempHit.t;
      outResult = tempHit;
      
      hit = true;
   }
   
   return hit;
}

bool ServerMap::Intersect(const SE::Physics::Collider& other, SE::Physics::CollisionResult& outResult) const
{
   bool collided = false;
   
   std::vector<uint32> candidateIds;
   candidateIds.reserve(64);
   
   spatial_.QueryAABB(other.GetWorldAABB(), candidateIds);
   
   SE::Physics::CollisionResult tempResult{};
   
   float maxPenetrationDepth = -std::numeric_limits<float>::max();
   
   for (uint32 colliderId : candidateIds) {
      const SE::Physics::Collider* collider = GetCollider(colliderId);
      if (!collider)
         continue;
      
      tempResult = {};
      if (!other.Intersect(collider->GetWorldAABB(), tempResult))
         continue;
      
      tempResult = {};
      if (!other.Intersect(*collider, tempResult))
         continue;
      
      if (!collided or tempResult.penetration > maxPenetrationDepth) {
         maxPenetrationDepth = tempResult.penetration;
         outResult = tempResult;
         collided = true;
      }
   }
   
   return collided;
}

bool ServerMap::SphereCast(const SE::Math::Vector3& from, const SE::Math::Vector3& to, float radius,
   SE::Physics::RaycastHit& outResult) const
{
   const SE::Math::Vector3 delta = to - from;
   const float dist = delta.Length();
   
   if (dist <= 0.001f)
      return false;
   
   bool hasHit = false;
   float closestT = std::numeric_limits<float>::max();
   
   const SE::Math::Vector3 min{ std::min(from.x, to.x) - radius, std::min(from.y, to.y) - radius, std::min(from.z, to.z) - radius };
   const SE::Math::Vector3 max{ std::max(from.x, to.x) + radius, std::max(from.y, to.y) + radius, std::max(from.z, to.z) + radius };
   SE::Physics::AABBCollider query{ min, max };
   
   std::vector<uint32> candidateIds;
   candidateIds.reserve(64);
   
   spatial_.QueryAABB(query, candidateIds);
   
   for (uint32 colliderId : candidateIds) {
      const auto* collider = GetCollider(colliderId);
      
      if (!collider)
         continue;
      
      SE::Physics::RaycastHit tempHit{};
      if (!collider->SphereCast(from, to, radius, tempHit))
         continue;
      
      if (!tempHit.hit)
         continue;
      
      if (tempHit.t < 0.0f or tempHit.t > dist)
         continue;
      
      if (tempHit.t >= closestT)
         continue;
      
      closestT = tempHit.t;
      outResult = tempHit;
      hasHit = true;
   }
   
   return hasHit;
}

bool ServerMap::FindNearestPoly(const SE::Math::Vector3& pos, const SE::Math::Vector3& halfExtents, dtPolyRef& outRef,
   SE::Math::Vector3& outNearest) const
{
   return navigation_.FindNearestPoly(pos, halfExtents, outRef, outNearest);
}

NavPathResult  ServerMap::FindPath(const SE::Math::Vector3& start, const SE::Math::Vector3& end,
   std::vector<SE::Math::Vector3>& outPath) const
{
   return navigation_.FindPath(start, end, outPath);
}

bool ServerMap::ProjectToNavMesh(const SE::Math::Vector3& pos, SE::Math::Vector3& outPos) const
{
   return navigation_.ProjectToNavMesh(pos, outPos);
}

bool ServerMap::MoveAlongSurface(const SE::Math::Vector3& start, const SE::Math::Vector3& end,
   SE::Math::Vector3& outPos) const
{
   return navigation_.MoveAlongSurface(start, end, outPos);
}

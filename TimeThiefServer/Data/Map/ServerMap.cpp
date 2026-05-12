#include "pch.h"
#include "ServerMap.h"
#include "Data/Loader/ServerMapLoader.h"
#include "Physics/Collider/Collider.h"
#include "Physics/Collider/CapsuleCollider.h"
#include "Physics/Collider/OBBCollider.h"
#include "Physics/Collider/SphereCollider.h"

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

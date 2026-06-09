#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "Physics/Hitbox/HitboxPart.h"

struct PawnCollisionPartData
{
   SE::Physics::Hit::HitShapeType shape{SE::Physics::Hit::HitShapeType::Sphere};
   SE::Physics::Hit::HitGroup group{SE::Physics::Hit::HitGroup::Unknown};
   float damageMultiplier{1.0f};

   SE::Math::Vector3 localOffset{};
   SE::Math::Vector3 localRotationDegrees{};
   SE::Math::Vector3 localPointA{};
   SE::Math::Vector3 localPointB{};
   SE::Math::Vector3 halfExtent{};
   float radius{0.0f};
   float halfHeight{0.0f};
};

struct PawnCollisionProfileDef
{
   uint32 collisionId{0};
   std::string pawnType;
   std::vector<PawnCollisionPartData> parts;

   bool IsValid() const
   {
      return collisionId != 0
         && !parts.empty();
   }
};

struct PawnCollisionProfileTable
{
   std::unordered_map<uint32, PawnCollisionProfileDef> profiles;

   const PawnCollisionProfileDef* GetProfile(uint32 collisionId) const
   {
      auto it = profiles.find(collisionId);
      if (it == profiles.end())
         return nullptr;

      return &it->second;
   }

   bool HasProfile(uint32 collisionId) const
   {
      return profiles.contains(collisionId);
   }

   bool IsValid() const
   {
      for (const auto& [_, profile] : profiles) {
         if (!profile.IsValid())
            return false;
      }

      return true;
   }
};

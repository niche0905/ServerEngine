#include "pch.h"
#include "PawnCollisionProfileTableJson.h"
#include <algorithm>
#include <fstream>
#include "json/json.h"

namespace
{
   using SE::Math::Vector3;
   using SE::Physics::Hit::HitGroup;
   using SE::Physics::Hit::HitShapeType;

   bool ReadVector3(const Json::Value& value, Vector3& out)
   {
      if (!value.isObject()
         || !value.isMember("x")
         || !value.isMember("y")
         || !value.isMember("z"))
         return false;

      if (!value["x"].isNumeric()
         || !value["y"].isNumeric()
         || !value["z"].isNumeric())
         return false;

      out = Vector3{
         value["x"].asFloat(),
         value["y"].asFloat(),
         value["z"].asFloat()
      };
      return true;
   }

   Vector3 RotateEulerXYZ(const Vector3& value, const Vector3& degrees)
   {
      const float rx = SE::Math::DegreesToRadians(degrees.x);
      const float ry = SE::Math::DegreesToRadians(degrees.y);
      const float rz = SE::Math::DegreesToRadians(degrees.z);

      Vector3 v = value;

      {
         const float c = std::cos(rx);
         const float s = std::sin(rx);
         v = Vector3{v.x, v.y * c - v.z * s, v.y * s + v.z * c};
      }
      {
         const float c = std::cos(ry);
         const float s = std::sin(ry);
         v = Vector3{v.x * c + v.z * s, v.y, -v.x * s + v.z * c};
      }
      {
         const float c = std::cos(rz);
         const float s = std::sin(rz);
         v = Vector3{v.x * c - v.y * s, v.x * s + v.y * c, v.z};
      }

      return v;
   }

   bool HasFlag(const Json::Value& row, const std::string& flag)
   {
      if (!row.isMember("flags") || !row["flags"].isArray())
         return false;

      for (const Json::Value& value : row["flags"]) {
         if (value.isString() && value.asString() == flag)
            return true;
      }

      return false;
   }

   HitGroup ParseHitGroup(const std::string& part)
   {
      if (part == "head")
         return HitGroup::Head;

      if (part == "body" || part == "torso")
         return HitGroup::Torso;

      if (part == "arm" || part == "arms" || part == "leftarm" || part == "rightarm")
         return HitGroup::Arms;

      if (part == "leg" || part == "legs" || part == "leftleg" || part == "rightleg")
         return HitGroup::Legs;

      return HitGroup::Unknown;
   }

   float DefaultDamageMultiplier(HitGroup group)
   {
      switch (group)
      {
      case HitGroup::Head:
         return 2.0f;
      case HitGroup::Arms:
      case HitGroup::Legs:
         return 0.75f;
      case HitGroup::Torso:
      case HitGroup::Unknown:
         return 1.0f;
      case HitGroup::NotHurtBox:
         return 0.0f;
      }

      return 1.0f;
   }

   bool ParseShape(const Json::Value& row, HitShapeType& out)
   {
      if (!row.isMember("shape") || !row["shape"].isString())
         return false;

      const std::string shape = row["shape"].asString();
      if (shape == "sphere") {
         out = HitShapeType::Sphere;
         return true;
      }
      if (shape == "capsule") {
         out = HitShapeType::Capsule;
         return true;
      }
      if (shape == "obb" || shape == "box") {
         out = HitShapeType::OBB;
         return true;
      }

      return false;
   }

   bool ParseCollider(const Json::Value& row, PawnCollisionPartData& out, std::string* outError)
   {
      if (!row.isObject()) {
         if (outError) *outError = "Collider row must be object";
         return false;
      }

      if (!ParseShape(row, out.shape)) {
         if (outError) *outError = "Missing or invalid collider shape";
         return false;
      }

      if (!row.isMember("local_position") || !ReadVector3(row["local_position"], out.localOffset)) {
         if (outError) *outError = "Missing or invalid local_position";
         return false;
      }

      if (row.isMember("local_rotation") && !ReadVector3(row["local_rotation"], out.localRotationDegrees)) {
         if (outError) *outError = "Invalid local_rotation";
         return false;
      }

      const std::string part = row.isMember("part") && row["part"].isString()
         ? row["part"].asString()
         : "";

      out.group = ParseHitGroup(part);
      out.damageMultiplier = DefaultDamageMultiplier(out.group);

      if (row.isMember("damage_multiplier") && row["damage_multiplier"].isNumeric())
         out.damageMultiplier = row["damage_multiplier"].asFloat();

      switch (out.shape)
      {
      case HitShapeType::Sphere:
         if (!row.isMember("radius") || !row["radius"].isNumeric()) {
            if (outError) *outError = "Sphere collider requires radius";
            return false;
         }
         out.radius = row["radius"].asFloat();
         if (out.radius <= 0.0f) {
            if (outError) *outError = "Sphere radius must be positive";
            return false;
         }
         break;

      case HitShapeType::Capsule:
         if (!row.isMember("radius") || !row["radius"].isNumeric()
            || !row.isMember("half_height") || !row["half_height"].isNumeric()) {
            if (outError) *outError = "Capsule collider requires radius and half_height";
            return false;
         }
         out.radius = row["radius"].asFloat();
         out.halfHeight = row["half_height"].asFloat();
         if (out.radius <= 0.0f || out.halfHeight <= 0.0f) {
            if (outError) *outError = "Capsule radius and half_height must be positive";
            return false;
         }
         {
            const Vector3 axis = RotateEulerXYZ(Vector3::Up(), out.localRotationDegrees).Normalized(Vector3::Up());
            const float segmentHalfHeight = std::max(0.0f, out.halfHeight - out.radius);
            out.localPointA = out.localOffset - axis * segmentHalfHeight;
            out.localPointB = out.localOffset + axis * segmentHalfHeight;
         }
         break;

      case HitShapeType::OBB:
         if (!row.isMember("half_extent") || !ReadVector3(row["half_extent"], out.halfExtent)) {
            if (outError) *outError = "OBB collider requires half_extent";
            return false;
         }
         break;
      }

      return true;
   }
}

namespace PawnCollisionProfileTableJson
{
   bool LoadFromFile(const std::filesystem::path& filePath, PawnCollisionProfileTable& outTable, std::string* outError)
   {
      std::ifstream file(filePath);
      if (!file.is_open()) {
         if (outError) *outError = "Failed to open file";
         return false;
      }

      Json::CharReaderBuilder builder;
      builder["collectComments"] = false;

      Json::Value root;
      std::string errs;
      if (!Json::parseFromStream(builder, file, &root, &errs)) {
         if (outError) *outError = errs;
         return false;
      }

      if (!root.isObject() || !root.isMember("pawn_collision_profiles") || !root["pawn_collision_profiles"].isArray()) {
         if (outError) *outError = "Missing or invalid pawn_collision_profiles";
         return false;
      }

      PawnCollisionProfileTable newTable;

      for (const Json::Value& row : root["pawn_collision_profiles"]) {
         if (!row.isObject()
            || !row.isMember("collision_id") || !row["collision_id"].isUInt()
            || !row.isMember("pawn_type") || !row["pawn_type"].isString()
            || !row.isMember("colliders") || !row["colliders"].isArray()) {
            if (outError) *outError = "Invalid pawn collision profile format";
            return false;
         }

         PawnCollisionProfileDef profile;
         if (!row["collision_id"].isUInt()) {
            if (outError) *outError = "collision_id must be uint";
            return false;
         }

         profile.collisionId = row["collision_id"].asUInt();
         profile.pawnType = row["pawn_type"].asString();

         profile.parts.reserve(row["colliders"].size());
         for (const Json::Value& colliderRow : row["colliders"]) {
            if (!HasFlag(colliderRow, "damage_receiver"))
               continue;

            PawnCollisionPartData part;
            if (!ParseCollider(colliderRow, part, outError))
               return false;

            profile.parts.push_back(part);
         }

         if (!profile.IsValid()) {
            if (outError) *outError = "Pawn collision profile validation failed: " + std::to_string(profile.collisionId);
            return false;
         }

         if (newTable.profiles.contains(profile.collisionId)) {
            if (outError) *outError = "Duplicate collision_id: " + std::to_string(profile.collisionId);
            return false;
         }

         newTable.profiles.emplace(profile.collisionId, std::move(profile));
      }

      outTable = std::move(newTable);
      return true;
   }
}

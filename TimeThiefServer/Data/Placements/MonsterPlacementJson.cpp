#include "pch.h"
#include "MonsterPlacementJson.h"
#include <fstream>
#include "json/json.h"

namespace
{
   bool ReadVector3(const Json::Value& value, SE::Math::Vector3& outPosition, std::string* outError)
   {
      if (!value.isObject() || !value.isMember("x") || !value.isMember("y") || !value.isMember("z")) {
         if (outError) *outError = "Invalid position format";
         return false;
      }

      if (!value["x"].isNumeric() || !value["y"].isNumeric() || !value["z"].isNumeric()) {
         if (outError) *outError = "Position x/y/z must be numeric";
         return false;
      }

      outPosition = SE::Math::Vector3{value["x"].asFloat(), value["y"].asFloat(), value["z"].asFloat()};
      return true;
   }

   bool ReadTransform(const Json::Value& value, PlacementTransform& outTransform, std::string* outError)
   {
      if (!value.isObject() || !value.isMember("position")) {
         if (outError) *outError = "Invalid transform format";
         return false;
      }

      if (!ReadVector3(value["position"], outTransform.position, outError))
         return false;

      if (value.isMember("yaw")) {
         if (!value["yaw"].isNumeric()) {
            if (outError) *outError = "Transform yaw must be numeric";
            return false;
         }

         outTransform.yaw = value["yaw"].asFloat();
      }

      return true;
   }

   bool ResolveMonsterTemplateId(const Json::Value& group, uint32& outTemplateId, std::string* outError)
   {
      if (group.isMember("template_id")) {
         if (!group["template_id"].isUInt()) {
            if (outError) *outError = "Monster template_id must be unsigned integer";
            return false;
         }

         outTemplateId = group["template_id"].asUInt();
         return true;
      }

      if (!group.isMember("type") || !group["type"].isString()) {
         if (outError) *outError = "Monster group must have string 'type' or unsigned integer 'template_id'";
         return false;
      }

      const std::string type = group["type"].asString();
      if (type == "cat") {
         outTemplateId = 2;
         return true;
      }

      if (outError) *outError = "Unknown monster type: " + type;
      return false;
   }

   bool ReadIsBossGroup(const Json::Value& group)
   {
      if (group.isMember("is_boss") && group["is_boss"].isBool()) {
         return group["is_boss"].asBool();
      }

      if (group.isMember("type") && group["type"].isString()) {
         return group["type"].asString() == "boss";
      }

      return false;
   }
}

namespace MonsterPlacementJson
{
   bool LoadFromFile(const std::filesystem::path& filePath, MonsterPlacementData& outData, std::string* outError)
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

      if (!root.isObject()) {
         if (outError) *outError = "Root must be object";
         return false;
      }

      if (!root.isMember("monster_spawn_groups") || !root["monster_spawn_groups"].isArray()) {
         if (outError) *outError = "Missing or invalid 'monster_spawn_groups'";
         return false;
      }

      outData.spawnGroups.clear();
      outData.spawnGroups.reserve(root["monster_spawn_groups"].size());

      for (const Json::Value& group : root["monster_spawn_groups"]) {
         if (!group.isObject() || !group.isMember("transform") || !group["transform"].isArray()) {
            if (outError) *outError = "Invalid monster spawn group format";
            return false;
         }

         uint32 templateId = 0;
         if (!ResolveMonsterTemplateId(group, templateId, outError))
            return false;

         const Json::Value& transforms = group["transform"];
         MonsterSpawnGroupPlacement placementGroup;
         placementGroup.templateId = templateId;
         placementGroup.spawnCount = static_cast<uint32>(transforms.size());
         placementGroup.isBoss = ReadIsBossGroup(group);

         if (group.isMember("spawn_num")) {
            if (!group["spawn_num"].isUInt()) {
               if (outError) *outError = "Monster spawn_num must be unsigned integer";
               return false;
            }

            placementGroup.spawnCount = group["spawn_num"].asUInt();
         }

         placementGroup.spawnCandidates.reserve(transforms.size());
         for (const Json::Value& transform : transforms) {
            PlacementTransform placementTransform;
            if (!ReadTransform(transform, placementTransform, outError))
               return false;

            placementGroup.spawnCandidates.push_back(placementTransform);
         }

         outData.spawnGroups.push_back(std::move(placementGroup));
      }

      return true;
   }
}

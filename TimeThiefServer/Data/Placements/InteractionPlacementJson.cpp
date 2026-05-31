#include "pch.h"
#include "InteractionPlacementJson.h"
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
}

namespace InteractionPlacementJson
{
   bool LoadFromFile(const std::filesystem::path& filePath, InteractionPlacementData& outData, std::string* outError)
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

      if (!root.isMember("store_spawn_locations") || !root["store_spawn_locations"].isArray()) {
         if (outError) *outError = "Missing or invalid 'store_spawn_locations'";
         return false;
      }

      if (!root.isMember("chest_spawn_locations") || !root["chest_spawn_locations"].isArray()) {
         if (outError) *outError = "Missing or invalid 'chest_spawn_locations'";
         return false;
      }

      outData.stores.clear();
      outData.chests.clear();
      outData.stores.reserve(root["store_spawn_locations"].size());
      outData.chests.reserve(root["chest_spawn_locations"].size());

      for (const Json::Value& value : root["store_spawn_locations"]) {
         StorePlacement placement;
         if (!ReadTransform(value, placement.transform, outError))
            return false;

         outData.stores.push_back(placement);
      }

      for (const Json::Value& value : root["chest_spawn_locations"]) {
         ChestPlacement placement;
         if (!ReadTransform(value, placement.transform, outError))
            return false;

         if (value.isMember("loot_table_id")) {
            if (!value["loot_table_id"].isInt()) {
               if (outError) *outError = "Chest loot_table_id must be integer";
               return false;
            }

            placement.lootTableId = value["loot_table_id"].asInt();
         }
         else if (value.isMember("table_id")) {
            if (!value["table_id"].isInt()) {
               if (outError) *outError = "Chest table_id must be integer";
               return false;
            }

            placement.lootTableId = value["table_id"].asInt();
         }

         outData.chests.push_back(placement);
      }

      return true;
   }
}

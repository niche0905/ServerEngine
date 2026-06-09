#include "pch.h"
#include "MonsterTemplateTableJson.h"
#include <fstream>
#include "json/json.h"

namespace
{
   bool HasRequiredMonsterFields(const Json::Value& row)
   {
      return row.isObject()
         && row.isMember("template_id")
         && row.isMember("name")
         && row.isMember("max_hp")
         && row.isMember("drop_point")
         && row.isMember("respawn_time")
         && row.isMember("loot_table")
         && row.isMember("collision_profile_id");
   }
}

namespace MonsterTemplateTableJson
{
   bool LoadFromFile(const std::filesystem::path& filePath, MonsterTemplateTable& outTable, std::string* outError)
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

      if (!root.isMember("monsters") || !root["monsters"].isArray()) {
         if (outError) *outError = "Missing or invalid 'monsters'";
         return false;
      }

      MonsterTemplateTable newTable;
      newTable.templates.reserve(root["monsters"].size());

      for (const Json::Value& row : root["monsters"]) {
         if (!HasRequiredMonsterFields(row)) {
            if (outError) *outError = "Invalid monster template format";
            return false;
         }

         if (!row["template_id"].isUInt()
            || !row["name"].isString()
            || !row["max_hp"].isInt()
            || !row["drop_point"].isInt()
            || !row["respawn_time"].isInt()
            || !row["loot_table"].isInt()
            || !row["collision_profile_id"].isUInt()) {
            if (outError) *outError = "Invalid monster template field type";
            return false;
         }

         MonsterTemplateDef monsterTemplate;
         monsterTemplate.templateId = row["template_id"].asUInt();
         monsterTemplate.name = row["name"].asString();
         monsterTemplate.maxHp = row["max_hp"].asInt();
         monsterTemplate.dropPoint = row["drop_point"].asInt();
         monsterTemplate.respawnTimeSec = row["respawn_time"].asInt();
         monsterTemplate.lootTableId = row["loot_table"].asInt();
         monsterTemplate.collisionProfileId = row["collision_profile_id"].asUInt();

         if (!monsterTemplate.IsValid()) {
            if (outError) *outError = "Monster template validation failed: " + std::to_string(monsterTemplate.templateId);
            return false;
         }

         if (newTable.templates.contains(monsterTemplate.templateId)) {
            if (outError) *outError = "Duplicate monster template_id: " + std::to_string(monsterTemplate.templateId);
            return false;
         }

         newTable.templates.emplace(monsterTemplate.templateId, std::move(monsterTemplate));
      }

      outTable = std::move(newTable);
      return true;
   }
}

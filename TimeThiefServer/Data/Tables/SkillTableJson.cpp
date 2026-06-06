#include "pch.h"
#include "SkillTableJson.h"
#include <fstream>
#include "json/json.h"

namespace SkillTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, SkillTable& outTable, std::string* outError)
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

        if (!root.isMember("skills") || !root["skills"].isArray()) {
            if (outError) *outError = "Missing or invalid 'skills'";
            return false;
        }

        outTable.skills.clear();

        for (const auto& row : root["skills"]) {
            if (!row.isObject()
                || !row.isMember("skill_id")
                || !row.isMember("cooldown_ms")
                || !row.isMember("duration_ms")
                || !row.isMember("cooldown_group_id")) {
                if (outError) *outError = "Invalid skill format";
                return false;
            }

            SkillDef def{};
            def.skillId = row["skill_id"].asUInt();
            def.cooldownMs = row["cooldown_ms"].asUInt();
            def.durationMs = row["duration_ms"].asUInt();
            def.cooldownGroupId = row["cooldown_group_id"].asUInt();

            if (def.skillId == 0) {
                if (outError) *outError = "skill_id must be non-zero";
                return false;
            }

            if (outTable.skills.contains(def.skillId)) {
                if (outError) *outError = "Duplicate skill_id";
                return false;
            }

            outTable.skills.emplace(def.skillId, def);
        }

        return true;
    }
}

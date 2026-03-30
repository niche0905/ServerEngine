#include "pch.h"
#include "ZoneTableJson.h"
#include <fstream>
#include <limits>
#include <json/json.h>

namespace
{
    Json::Value ZonePhaseDataToJson(const ZonePhaseData& phase)
    {
        Json::Value value(Json::objectValue);
        value["radius"] = phase.radius;
        value["damagePerSecond"] = phase.damagePerSecond;
        value["waitTimeSeconds"] = phase.waitTimeSeconds;
        value["shrinkTimeSeconds"] = phase.shrinkTimeSeconds;
        return value;
    }

    bool ZonePhaseDataFromJson(const Json::Value& value, ZonePhaseData& outPhase, std::string* outError)
    {
        if (!value.isObject())
        {
            if (outError) *outError = "ZonePhaseData entry is not an object.";
            return false;
        }
        
        if (!value.isMember("radius") || !value["radius"].isNumeric())
        {
            if (outError) *outError = "Missing or invalid 'radius'.";
            return false;
        }

        if (!value.isMember("damagePerSecond") || !value["damagePerSecond"].isNumeric())
        {
            if (outError) *outError = "Missing or invalid 'damagePerSecond'.";
            return false;
        }

        if (!value.isMember("waitTimeSeconds") || !value["waitTimeSeconds"].isNumeric())
        {
            if (outError) *outError = "Missing or invalid 'waitTimeSeconds'.";
            return false;
        }

        if (!value.isMember("shrinkTimeSeconds") || !value["shrinkTimeSeconds"].isNumeric())
        {
            if (outError) *outError = "Missing or invalid 'shrinkTimeSeconds'.";
            return false;
        }

        outPhase.radius = value["radius"].asFloat();
        outPhase.damagePerSecond = value["damagePerSecond"].asFloat();
        outPhase.waitTimeSeconds = value["waitTimeSeconds"].asFloat();
        outPhase.shrinkTimeSeconds = value["shrinkTimeSeconds"].asFloat();

        return true;
    }

    Json::Value ZoneTableToJson(const ZoneTable& table)
    {
        Json::Value root(Json::objectValue);
        Json::Value phases(Json::arrayValue);

        for (const ZonePhaseData& phase : table.phases)
        {
            phases.append(ZonePhaseDataToJson(phase));
        }

        root["phases"] = phases;
        return root;
    }

    bool ZoneTableFromJson(const Json::Value& root, ZoneTable& outTable, std::string* outError)
    {
        if (!root.isObject())
        {
            if (outError) *outError = "Root JSON is not an object.";
            return false;
        }

        const Json::Value& phases = root["phases"];
        if (!phases || !phases.isArray())
        {
            if (outError) *outError = "Missing or invalid 'phases' array.";
            return false;
        }

        ZoneTable tempTable;
        tempTable.phases.reserve(phases.size());

        for (Json::ArrayIndex i = 0; i < phases.size(); ++i)
        {
            ZonePhaseData phase;
            std::string phaseError;
            if (!ZonePhaseDataFromJson(phases[i], phase, &phaseError))
            {
                if (outError)
                {
                    *outError = "Failed to parse phases[" + std::to_string(i) + "]: " + phaseError;
                }
                return false;
            }

            tempTable.phases.push_back(phase);
        }

        outTable = std::move(tempTable);
        return true;
    }
}

namespace ZoneTableJson
{
    bool Validate(const ZoneTable& table, std::string* outError)
    {
        if (table.phases.empty())
        {
            if (outError) *outError = "ZoneTable.phases is empty.";
            return false;
        }

        float prevradius = std::numeric_limits<float>::max();

        for (size_t i = 0; i < table.phases.size(); ++i)
        {
            const ZonePhaseData& phase = table.phases[i];

            if (phase.radius <= 0.0f)
            {
                if (outError) *outError = "phases[" + std::to_string(i) + "].radius must be > 0.";
                return false;
            }

            if (phase.damagePerSecond < 0.0f)
            {
                if (outError) *outError = "phases[" + std::to_string(i) + "].damagePerSecond must be >= 0.";
                return false;
            }

            if (phase.waitTimeSeconds < 0.0f)
            {
                if (outError) *outError = "phases[" + std::to_string(i) + "].waitTimeSeconds must be >= 0.";
                return false;
            }

            if (phase.shrinkTimeSeconds < 0.0f)
            {
                if (outError) *outError = "phases[" + std::to_string(i) + "].shrinkTimeSeconds must be >= 0.";
                return false;
            }

            if (phase.radius > prevradius)
            {
                if (outError) *outError = "phases[" + std::to_string(i) + "].radius must not increase.";
                return false;
            }

            prevradius = phase.radius;
        }

        return true;
    }

    bool LoadFromFile(const std::filesystem::path& filePath, ZoneTable& outTable, std::string* outError)
    {
        std::ifstream ifs(filePath);
        if (!ifs.is_open())
        {
            if (outError) *outError = "Failed to open file: " + filePath.string();
            return false;
        }

        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;

        Json::Value root;
        std::string parseErrors;
        if (!Json::parseFromStream(builder, ifs, &root, &parseErrors))
        {
            if (outError) *outError = "JSON parse failed: " + parseErrors;
            return false;
        }

        ZoneTable tempTable;
        if (!ZoneTableFromJson(root, tempTable, outError))
        {
            return false;
        }

        std::string validateError;
        if (!Validate(tempTable, &validateError))
        {
            if (outError) *outError = "Validation failed: " + validateError;
            return false;
        }

        outTable = std::move(tempTable);
        return true;
    }

    bool SaveToFile(const std::filesystem::path& filePath, const ZoneTable& table, std::string* outError)
    {
        std::string validateError;
        if (!Validate(table, &validateError))
        {
            if (outError) *outError = "Validation failed: " + validateError;
            return false;
        }

        std::ofstream ofs(filePath);
        if (!ofs.is_open())
        {
            if (outError) *outError = "Failed to open file for write: " + filePath.string();
            return false;
        }

        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";

        Json::Value root = ZoneTableToJson(table);

        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(root, &ofs);

        return true;
    }
}
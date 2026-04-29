#include "pch.h"
#include "PlayerSpawnTableJson.h"
#include <fstream>
#include "json/json.h"

namespace PlayerSpawnTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, PlayerSpawnTable& outTable, std::string* outError)
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
        
        if (!root.isMember("spawnPoints") || !root["spawnPoints"].isArray()) {
            if (outError) *outError = "Missing or invalid 'spawnPoints'";
            return false;
        }
        
        outTable.spawnPoints.clear();
        outTable.spawnPoints.reserve(root["spawnPoints"].size());
        for (const Json::Value& point : root["spawnPoints"]) {
            if (!point.isObject() || !point.isMember("x") || !point.isMember("y") || !point.isMember("z")) {
                if (outError) *outError = "Invalid spawn point format";
                return false;
            }
            
            if (!point["x"].isNumeric() || !point["y"].isNumeric() || !point["z"].isNumeric()) {
                if (outError) *outError = "Spawn point x/y/z must be numeric";
                return false;
            }
            
            float x = point["x"].asFloat();
            float y = point["y"].asFloat();
            float z = point["z"].asFloat();
            outTable.spawnPoints.emplace_back(x, y, z);
        }
        
        if (outTable.spawnPoints.empty()) {
            if (outError) *outError = "No valid spawn points found";
            return false;
        }
        
        return true;
    }

}

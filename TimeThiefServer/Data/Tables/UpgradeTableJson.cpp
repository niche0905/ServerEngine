#include "pch.h"
#include "UpgradeTableJson.h"
#include <fstream>
#include "json/json.h"

namespace UpgradeTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, WeaponUpgradeTable& outTable, std::string* outError)
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
            if (outError) *outError = errs;
            return false;
        }
        
        if (!root.isMember("upgrades") || !root["upgrades"].isArray()) {
            if (outError) *outError = "Missing or invalid 'upgrades'";
            return false;
        }
        
        outTable.defs.clear();
        for (const auto& upgrade : root["upgrades"]) {
            if (!upgrade.isObject() || !upgrade.isMember("code") || !upgrade.isMember("target") || !upgrade.isMember("modifier")) {
                if (outError) *outError = "Invalid weapon upgrade format";
                return false;
            }
            
            uint32 code = upgrade["code"].asUInt();
            uint32 target = upgrade["target"].asUInt();
            
            WeaponUpgradeDef upgradeDef{};
            upgradeDef.target.weaponId = target;
            const auto& modifier = upgrade["modifier"];
            {
                if (modifier.isMember("damage_delta")) {
                    upgradeDef.modifier.damageDelta = modifier["damage_delta"].asInt();
                }
                
                if (modifier.isMember("mag_capacity_delta")) {
                    upgradeDef.modifier.magCapacityDelta = modifier["mag_capacity_delta"].asInt();
                }
                
                if (modifier.isMember("fire_interval_set_delta")) {
                    upgradeDef.modifier.fireIntervalSecDelta = modifier["fire_interval_set_delta"].asFloat();
                }
                
                if (modifier.isMember("reload_time_set_delta")) {
                    upgradeDef.modifier.reloadTimeSecDelta = modifier["reload_time_set_delta"].asFloat();
                }
                
                if (modifier.isMember("range_delta")) {
                    upgradeDef.modifier.rangeDelta = modifier["range_delta"].asFloat();
                }
                
                if (modifier.isMember("pallet_count_delta")) {
                    upgradeDef.modifier.palletCountDelta = modifier["pallet_count_delta"].asInt();
                }
                
                if (modifier.isMember("cone_angle_degrees_delta")) {
                    upgradeDef.modifier.coneAngleDegreesDelta = modifier["cone_angle_degrees_delta"].asFloat();
                }
                
                if (modifier.isMember("projectile_speed_delta")) {
                    upgradeDef.modifier.projectileSpeedDelta = modifier["projectile_speed_delta"].asFloat();
                }
                
                if (modifier.isMember("explosion_radius_delta")) {
                    upgradeDef.modifier.explosionRadiusDelta = modifier["explosion_radius_delta"].asFloat();
                }
            }
            
            outTable.defs[code] = upgradeDef;
        }
        
        return true;
    }

    bool LoadFromFile(const std::filesystem::path& filePath, StatUpgradeTable& outTable, std::string* outError)
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
            if (outError) *outError = errs;
            return false;
        }
        
        if (!root.isMember("upgrades") || !root["upgrades"].isArray()) {
            if (outError) *outError = "Missing or invalid 'upgrades'";
            return false;
        }
        
        outTable.statUpgrades.clear();
        for (const auto& stat : root["upgrades"]) {
            if (!stat.isObject() || !stat.isMember("code") || !stat.isMember("stat_infos") || !stat["stat_infos"].isArray()) {
                if (outError) *outError = "Invalid stat upgrade format";
                return false;
            }
            
            uint32 statCode = stat["code"].asUInt();
            StatUpgradeEntry statEntry;
            statEntry.statUpgrades.reserve(stat["stat_infos"].size());
            
            for (const auto& statInfo : stat["stat_infos"]) {
                if (!statInfo.isMember("delta")) {
                    if (outError) *outError = "Invalid stat delta";
                    return false;
                }
                
                int32 delta = statInfo["delta"].asInt();
                
                statEntry.statUpgrades.push_back({delta});
            }
            
            outTable.statUpgrades[statCode] = std::move(statEntry);
        }
        
        return true;
    }
}

#include "pch.h"
#include "WeaponTableJson.h"
#include <fstream>
#include "json/json.h"

namespace WeaponTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, WeaponTable& outTable, std::string* outError)
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
        
        if (!root.isMember("weapons") || !root["weapons"].isArray()) {
            if (outError) *outError = "Missing or invalid 'weapons'";
            return false;
        }
        
        outTable.tables.clear();
        for (const auto& row : root["weapons"]) {
            if (!row.isObject() || !row.isMember("weapon_id") || !row.isMember("category") || !row.isMember("fire_type") || !row.isMember("damage") || !row.isMember("magazine_size") || !row.isMember("fire_interval") || !row.isMember("reload_time") || !row.isMember("range")) {
                if (outError) *outError = "Invalid weapon standard format";
                return false;
            }
            
            WeaponStat stat;
            stat.common.category = static_cast<WeaponCategory>(row["category"].asUInt());
            stat.common.fireType = static_cast<WeaponFireType>(row["fire_type"].asUInt());
            stat.common.damage = row["damage"].asInt();
            stat.common.magCapacity = row["magazine_size"].asInt();
            stat.common.fireIntervalSec = row["fire_interval"].asFloat();
            stat.common.reloadTimeSec = row["reload_time"].asFloat();
            stat.common.range = row["range"].asFloat();
            
            switch (stat.common.category)
            {
            case WeaponCategory::Rifle:
                {
                    stat.extra = RifleStat{};
                }
                break;
                
            case WeaponCategory::Shotgun:
                {
                    if (!row.isMember("pellet_count") || !row.isMember("cone_angle"))
                    {
                        if (outError) *outError = "Missing shotgun-specific fields";
                        return false;
                    }
                
                    int pelletCount = row["pellet_count"].asInt();
                    float coneAngle = row["cone_angle"].asFloat();
                
                    stat.extra = ShotgunStat{pelletCount, coneAngle};
                }
                break;
                
            case WeaponCategory::Launcher:
                {
                    if (!row.isMember("projectile_speed") || !row["explosion_radius"])
                    {
                        if (outError) *outError = "Missing launcher-specific fields";
                        return false;
                    }
                
                    float projectileSpeed = row["projectile_speed"].asFloat();
                    float explosionRadius = row["explosion_radius"].asFloat();
                
                    stat.extra = LauncherStat{projectileSpeed, explosionRadius};
                }
                break;
            }
            
            outTable.tables[row["weapon_id"].asUInt()] = stat;
        }
        
        return true;
    }
}

#include "pch.h"
#include "StoreEntryTableJson.h"
#include <fstream>
#include "json/json.h"

namespace StoreEntryTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, StoreEntryTable& outTable, std::string* outError)
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
        
        if (!root.isMember("entries") || !root["entries"].isArray()) {
            if (outError) *outError = "Missing or invalid 'entries'";
            return false;
        }
        
        if (!root.isMember("upgrade_lines") || !root["upgrade_lines"].isArray()) {
            if (outError) *outError = "Missing or invalid 'upgrade_lines'";
            return false;
        }
        
        outTable.Entries.clear();
        outTable.UpgradeLines.clear();
        
        for (const auto& entry : root["entries"]) {
            if (!entry.isObject() || !entry.isMember("entry_id") || !entry.isMember("cost") || !entry.isMember("type")) {
                if (outError) *outError = "Invalid entry format";
                return false;
            }
            
            StoreEntryDef def;
            def.entryId = entry["entry_id"].asUInt();
            def.cost = entry["cost"].asInt();
            def.rewardType = static_cast<StoreRewardType>(entry["type"].asUInt());
            
            switch (def.rewardType)
            {
            case StoreRewardType::Item:
                {
                    if (!entry.isMember("item_id") || !entry.isMember("item_count")) {
                        if (outError) *outError = "Invalid item format";
                        return false;
                    }
                    
                    def.itemId = entry["item_id"].asUInt();
                    def.itemCount = entry["item_count"].asInt();
                }
                break;
                
            case StoreRewardType::Skill:
                {
                    if (!entry.isMember("skill_id")) {
                        if (outError) *outError = "Invalid skill format";
                        return false;
                    }
                    
                    def.skillId = entry["skill_id"].asUInt();
                }
                break;
                
            case StoreRewardType::WeaponUpgrade:
                {
                    if (!entry.isMember("weapon_upgrade_type")) {
                        if (outError) *outError = "Invalid weapon upgrade format";
                        return false;
                    }
                    
                    def.weaponUpgradeType = entry["weapon_upgrade_type"].asUInt();
                }
                break;
                
            case StoreRewardType::StatUpgrade:
                {
                    if (!entry.isMember("stat_upgrade_type")) {
                        if (outError) *outError = "Invalid stat upgrade format";
                        return false;
                    }
                    
                    def.statUpgradeType = entry["stat_upgrade_type"].asUInt();
                }
                break;
            }
            
            if (entry.isMember("upgrade_line_id")) {
                def.upgradeLineId = entry["upgrade_line_id"].asUInt();
            }
        }
        
        for (const auto& upgradeLine : root["upgrade_lines"]) {
            if (!upgradeLine.isObject() || !upgradeLine.isMember("line_id") || !upgradeLine.isMember("steps") || !upgradeLine["steps"].isArray()) {
                if (outError) *outError = "Invalid upgrade line format";
                return false;
            }
            
            UpgradeLineDef line;
            line.lienId = upgradeLine["line_id"].asUInt();
            const auto& steps = upgradeLine["steps"];
            line.steps.reserve(steps.size());
            
            for (const auto& step : steps) {
                if (!step.isObject() || !step.isMember("cost")) {
                    if (outError) *outError = "Invalid spawn step format";
                    return false;
                }
                
                int32 cost = step["cost"].asInt();
                line.steps.push_back({ cost });
            }
        }
        
        return true;
    }
}

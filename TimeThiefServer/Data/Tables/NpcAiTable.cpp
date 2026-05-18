#include "pch.h"
#include "NpcAiTable.h"
#include <fstream>
#include <sstream>
#include <json/json.h>

bool NpcAiTable::LoadFromFile(const std::filesystem::path& filePath, std::string* outError)
{
    entries_.clear();
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        if (outError) *outError = "Failed to open file: " + filePath.string();
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errs;

    std::istringstream iss(buffer.str());
    if (!Json::parseFromStream(builder, iss, &root, &errs)) {
        if (outError) *outError = "Failed to parse JSON: " + errs;
        return false;
    }
    
    if (!root.isArray()) {
        if (outError) *outError = "NpcAiTable must be a JSON array.";
        return false;
    }
    
    const std::filesystem::path baseDir = filePath.parent_path();
    
    for (const auto& elem : root) {
        
        if (!elem.isObject())
            continue;
        
        if (!elem.isMember("npc_id") or !elem["npc_id"].isUInt()) {
            if (outError) *outError = "Missing or invalid npc_id.";
            return false;
        }
        
        if (!elem.isMember("bt_xml") or !elem["bt_xml"].isString()) {
            if (outError) *outError = "Missing or invalid bt_xml.";
            return false;
        }
        
        NpcAiEntry entry;
        entry.npcId = elem["npc_id"].asUInt();
        
        if (elem.isMember("name") and elem["name"].isString())
            entry.name = elem["name"].asString();
        
        std::filesystem::path rawPath = elem["bt_xml"].asString();
        if (rawPath.is_absolute())
            entry.btXmlPath = rawPath.lexically_normal();
        else
            entry.btXmlPath = (baseDir / rawPath).lexically_normal();
        
        if (entries_.contains(entry.npcId)) {
            if (outError) *outError = "Duplicate npc_id: " + std::to_string(entry.npcId);
            return false;
        }
        
        entries_.emplace(entry.npcId, std::move(entry));
    }
    
    return true;
}

const NpcAiEntry* NpcAiTable::Find(uint32 npcId) const
{
    if (entries_.contains(npcId))
        return &entries_.at(npcId);
    
    return nullptr;
}

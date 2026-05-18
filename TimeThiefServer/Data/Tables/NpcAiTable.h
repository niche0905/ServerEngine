#pragma once
#include <filesystem>
#include <string>

struct NpcAiEntry
{
    uint32 npcId = 0;
    std::string name;
    std::string btId;  // Behavior Tree Factory에 등록된 ID
    std::filesystem::path btXmlPath;
};

class NpcAiTable
{
public:
    bool LoadFromFile(const std::filesystem::path& filePath, std::string* outError = nullptr);
    
    const NpcAiEntry* Find(uint32 npcId) const;
    const std::unordered_map<uint32, NpcAiEntry>& GetAll() const { return entries_; }
    
private:
    std::unordered_map<uint32, NpcAiEntry> entries_;
    
};

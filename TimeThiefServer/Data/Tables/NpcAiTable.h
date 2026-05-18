#pragma once
#include <filesystem>
#include <string>

struct NpcAiEntry
{
    uint32 npcId = 0;
    std::string name;
    std::filesystem::path btXmlPath;
};

class NpcAiTable
{
public:
    bool LoadFromFile(const std::filesystem::path& filePath, std::string* outError = nullptr);
    
    const NpcAiEntry* Find(uint32 npcId) const;
    
private:
    std::unordered_map<uint32, NpcAiEntry> entries_;
    
};

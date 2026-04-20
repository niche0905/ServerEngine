#pragma once
#include <unordered_map>

struct StoreEntry
{
    uint32 EntryId;
    int64 Cost;
};

struct StoreEntryTable
{
    std::unordered_map<uint32, StoreEntry> Entries;
    
    int64 GetCost(uint32 entryId) const
    {
        auto it = Entries.find(entryId);
        if (it != Entries.end())
            return it->second.Cost;
        
        return -1;  // 유효하지 않은 entryId
    }
    
    bool IsValidEntry(uint32 entryId) const
    {
        return Entries.contains(entryId);
    }
};

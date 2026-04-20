#pragma once
#include <unordered_map>
#include <Content/Gameplay/Economy/StoreTypes.h>

struct StoreEntryDef;

struct StoreEntryTable
{
    std::unordered_map<uint32, StoreEntryDef> Entries;
    
    int64 GetCost(uint32 entryId) const
    {
        auto it = Entries.find(entryId);
        if (it != Entries.end())
            return it->second.cost;
        
        return -1;  // 유효하지 않은 entryId
    }
    
    const StoreEntryDef* GetStoreEntry(uint32 entryId) const
    {
        if (!Entries.contains(entryId))
            return nullptr;
       
        return &Entries.at(entryId);
    }
    
    bool IsValidEntry(uint32 entryId) const
    {
        return Entries.contains(entryId);
    }
};

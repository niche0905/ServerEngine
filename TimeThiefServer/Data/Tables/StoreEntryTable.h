#pragma once
#include <unordered_map>
#include <Content/Gameplay/Economy/StoreTypes.h>

struct StoreEntryDef;

struct StoreEntryTable
{
    std::unordered_map<uint32, StoreEntryDef> Entries;
    std::unordered_map<uint32, UpgradeLineDef> UpgradeLines; 
    
    int32 GetCost(uint32 entryId, uint32 nowLevel = 0) const
    {
        auto it = Entries.find(entryId);
        if (it == Entries.end())
            return -1;
        
        if (it->second.upgradeLineId == 0)
            return it->second.cost;
        
        auto lineIt = UpgradeLines.find(it->second.upgradeLineId);
        if (lineIt == UpgradeLines.end())
            return -1;
        
        return lineIt->second.GetStep(nowLevel)->cost;
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
    
    int32 GetMaxLevel(uint32 lineId) const
    {
        auto lineIt = UpgradeLines.find(lineId);
        if (lineIt == UpgradeLines.end())
            return -1;
        
        return static_cast<int32>(lineIt->second.steps.size());
    }
    
};

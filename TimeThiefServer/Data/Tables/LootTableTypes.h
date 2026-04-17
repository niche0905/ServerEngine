#pragma once
#include "Content/Gameplay/Inventory/ItemTypes.h"
#include "Content/Gameplay/Economy/WalletTypes.h"
#include "Content/Gameplay/Loot/LootTypes.h"
#include <string>
#include <vector>
#include "Content/Object/ObjectId.h"
#include "Utils/Random/WeightedRandom.h"

inline int64 RollRangeI64(int64 minInclusive, int64 maxInclusive, Random32& rng);


struct IntRange
{
    int32 min{0};
    int32 max{0};
    
    bool IsValid() const { return min <= max; }
};

struct Int64Range
{
    int64 min{0};
    int64 max{0};
    
    bool IsValid() const { return min <= max; }
};

struct Chance   // 0.0 ~ 1.0
{
    double value{1.0};
    bool IsValid() const { return value >= 0.0 and value <= 1.0; }
};

struct LootEntry
{
    ItemId itemId{0};
    IntRange itemCount{1, 1};
    
    CurrencyType currencyId{CurrencyType::None};
    Int64Range moneyAmount{0, 0};
    
    int32 weight{1};
    
    Chance chance{1.0};
    
    bool IsItem() const { return itemId != 0; }
    bool IsMoney() const { return currencyId == CurrencyType::TimePoint; }
    bool IsValid() const
    {
        if (weight <= 0) return false;
        if (not chance.IsValid()) return false;
        
        if (IsItem()) return itemCount.IsValid() and itemCount.max > 0;
        if (IsMoney()) return moneyAmount.IsValid() and moneyAmount.max > 0;
        
        return false;
    }
};

struct LootGroup
{
    std::string id; // debugging 용
    Chance chance{1.0}; // 그룹이 선택될 확률
    
    IntRange pickCount{0, 0}; // 이 그룹에서 선택할 항목 수 (0이면 모두 선택)
    
    bool allowDuplicates{false}; // 중복 선택 허용 여부
    
    std::vector<LootEntry> entries;
    
    bool IsValid() const
    {
        if (not chance.IsValid()) return false;
        if (not pickCount.IsValid()) return false;
        if (pickCount.max < 0) return false;
        if (entries.empty()) return true;   // 빈 그룹 허용
        for (const auto e : entries) {
            if (not e.IsValid()) return false;
        }
        
        return true;
    }
};

struct LootTableDef // 몬스터, 상자 등의 루팅 테이블 정의
{
    int32 tableId{0};

    std::vector<LootGroup> groups;
    // TODO: 고정 보상 항목 추가
    
    bool IsValid() const
    {
        if (tableId <= 0) return false;
        for (const auto& g : groups) {
            if (not g.IsValid()) return false;
        }
       
        return true;
    }
};

struct LootTable
{
    std::unordered_map<int32, LootTableDef> tables;
    
    bool IsValid() const
    {
        for (const auto& pair : tables) {
            if (not pair.second.IsValid()) return false;
        }
        return true;
    }
    
    bool HasTable(int32 tableId) const
    {
        return tables.contains(tableId);
    }
    
    const LootTableDef* GetTable(int32 tableId) const
    {
        if (not HasTable(tableId)) return nullptr;
        return &tables.at(tableId);
    }
    
    LootBundle Roll(int32 tableId, uint32 rngSeed/*const LootRollContext& ctx*/) const
    {
        LootBundle out;
   
        auto it = tables.find(tableId);
        if (it == tables.end())
            return out;
   
        const LootTableDef& table = it->second;
   
        Random32 rng{rngSeed};
   
        std::vector<int32> picked;
        for (const auto& group : table.groups) {
      
            if (not rng.Chance(group.chance.value))
                continue;   // 그룹 출현 실패
      
            const int32 k = rng.NextI32(group.pickCount.min, group.pickCount.max);
            if (k <= 0)
                continue;   // 선택 개수 없음
      
            picked.clear();
            ChooseManyIndicesByWeight(static_cast<int32>(group.entries.size()), k, group.allowDuplicates, 
               [&](int32 i) { return group.entries[static_cast<size_t>(i)].weight; },
               rng, picked);
      
            for (int32 idx : picked) {
                if (idx < 0 or static_cast<size_t>(idx) >= group.entries.size())
                    continue;
         
                const auto& entry = group.entries[static_cast<size_t>(idx)];
         
                if (not rng.Chance(entry.chance.value))
                    continue;
         
                if (entry.IsItem()) {
                    const int32 count = rng.NextI32(entry.itemCount.min, entry.itemCount.max);
                    if (count > 0) {
                        out.AddItem(entry.itemId, count);
                    }
                }
                else if (entry.IsMoney()) {
                    const int64 amount = RollRangeI64(entry.moneyAmount.min, entry.moneyAmount.max, rng);
                    if (amount > 0) {
                        out.AddMoney(entry.currencyId, amount);
                    }
                }
            }
        }
   
        return out;
    }
};

struct LootSourceContext
{
    ObjectId owner;      // 드랍 주체
    ObjectId killer;     // 드랍 처치자
    uint32 rngSeed{0};  // 드랍 시드 (재현 가능하게)
};

inline std::ostream& operator<<(std::ostream& os, const LootEntry& entry)
{
    os << "itemId: " << entry.itemId
        << ", weight: " << entry.weight
        << ", chance: " << entry.chance.value;
    
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const LootGroup& group)
{
    os << "group\n";
    for (const auto& e : group.entries) {
        os << e << "\n";
    }
    
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const LootTableDef& ltd)
{
    os << "table id: " << ltd.tableId << "\n";
    for (const auto& g : ltd.groups) {
        os << g << "\n";
    }
    
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const LootTable& table)
{
    os << "LootTable:\n";
    for (const auto& [key, def] : table.tables) {
        os << def << "\n";
    }
    
    return os;
}

inline int64 RollRangeI64(int64 minInclusive, int64 maxInclusive, Random32& rng)
{
    if (minInclusive >= maxInclusive)
        return minInclusive;
    const uint64 span = static_cast<uint64>(maxInclusive - minInclusive) + 1ull;
      
    // 범위가 32비트 이내인 경우
    if (span <= 0xFFFFFFFFull) {
        const uint32 r = rng.NextU32(static_cast<uint32>(span));
        return minInclusive + static_cast<int64>(r);
    }
      
    // 64비트 범위인 경우
    const uint64 r64 = (static_cast<uint64>(rng.NextU32()) << 32) | static_cast<uint64>(rng.NextU32());
    return minInclusive + static_cast<int64>(r64 % span);
}

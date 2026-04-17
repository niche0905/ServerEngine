#pragma once
#include "Content/Gameplay/Inventory/ItemTypes.h"
#include "Content/Gameplay/Economy/WalletTypes.h"
#include "Content/Gameplay/Loot/LootTypes.h"
#include <string>
#include <vector>

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
    
    CurrencyType currencyId{0};
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

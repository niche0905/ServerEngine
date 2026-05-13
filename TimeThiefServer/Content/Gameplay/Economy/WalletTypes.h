#pragma once

using CurrencyId = uint16;
using CurrencyAmount = int32;

enum class CurrencyType : CurrencyId
{
    None = 0,
    
    TimePoint = 1,
    // Gold,
    // Silver,
    // Gem,
};

enum class MoneyChangeReason : uint8
{
    Unknown = 0,
    Loot,
    DropOnDeath,
    // QuestReward,
    Purchase,
    RespawnCost,
    // Sell,
    Cheat,
    System,
    ZoneDamage,
};

struct MoneyChangeContext
{
    MoneyChangeReason reason{MoneyChangeReason::Unknown};

    // 선택적 추적 정보
    uint32 relatedItemId{0};
    // ObjectId source; // 필요하면 공격자/몬스터 등
};

struct MoneyChangeResult
{
    CurrencyType currency{0};
    CurrencyAmount before{0};
    CurrencyAmount after{0};
    CurrencyAmount delta{0};

    bool accepted{false}; // 변경이 실제로 적용되었는지
};

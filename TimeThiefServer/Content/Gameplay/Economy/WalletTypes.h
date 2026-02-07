#pragma once

using CurrencyId = uint16;

enum class CurrencyType : CurrencyId
{
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
    // Sell,
    Cheat,
    System,
};

struct MoneyChangeContext
{
    MoneyChangeReason reason{MoneyChangeReason::Unknown};

    // 선택적 추적 정보
    int32 relatedItemId{0};
    // ObjectId source; // 필요하면 공격자/몬스터 등
};

struct MoneyChangeResult
{
    CurrencyId currency{0};
    int64 before{0};
    int64 after{0};
    int64 delta{0};

    bool accepted{false}; // 변경이 실제로 적용되었는지
};

#pragma once
#include "Content/Gameplay/Inventory/ItemTypes.h"
#include "Content/Gameplay/Economy/WalletTypes.h"
#include <vector>

struct ItemStack;
struct MoneyDrop
{
    CurrencyType currency{0};
    int64 amount{0};
};

/*--------------
   LootBundle
--------------*/
//
// LootBundle는 획득한 아이템과 화폐의 묶음을 나타냅니다.
//

struct LootBundle
{
    std::vector<ItemStack> items;
    std::vector<MoneyDrop> money;
    
    bool Empty() const { return items.empty() and money.empty(); }
    void Clear() { items.clear(); money.clear(); }
    
    void AddItem(ItemId id, int32 count)
    {
        if (id == 0 or count <= 0) return;
        items.push_back(ItemStack{ id, count });
    }
    
    void AddMoney(CurrencyType currency, int64 amount)
    {
        if (currency == CurrencyType::None or amount <= 0) return;
        money.push_back(MoneyDrop{ currency, amount });
    }
    
};

// MEMO: 추가적인 룰렛 컨텍스트 정보가 필요할 경우 여기에 확장합니다. (ex. 레벨 보정 혹은 럭 요인)
struct LootRollContext
{
    uint32 monsterLevel{1};
    uint32 playerLevel{1};
    float luckFactor{1.0f};
};

struct LootResult
{
    bool generated{false};
    LootBundle bundle;
};

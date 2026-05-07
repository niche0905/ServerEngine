#pragma once
#include "TypesDef.h"
#include "WalletTypes.h"
#include "Content/Object/ObjectId.h"

class Actor;
class PlayerPawn;

struct StoreBuyRequest
{
    ObjectId playerId;
    ObjectId storeId;
    uint32 entryId;
};

enum class StoreBuyResultCode : uint8
{
    Success = 0,
    
    InvalidPlayer,                  // Player가 아님
    InvalidStore,                   // 상점이 아님
    InvalidEntry,                   // 유효한 EntryId가 아님 (없는 EntryId거나, 해당 상점에 없는 EntryId)
    TooFar,                         // 플레이어가 상점에서 너무 멀리 떨어져 있음
    InvalidState,                   // 구매 시점에 플레이어가 구매할 수 없는 상태 (예: 사망한 상태, ...)
    NotEnoughCurrency,              // 플레이어가 구매에 필요한 화폐를 충분히 가지고 있지 않음
    PurchaseLimitExceeded,          // 구매 한도 초과 (예: 하루 구매 한도, 아이템별 구매 한도 등)
    ApplyFailed,                    // 구매 적용에 실패 (예: 인벤토리 공간 부족, 아이템 적용 중 오류 등)
};

enum class StoreRewardType : uint8
{
    None = 0,
    
    Item,                           // 아이템 보상
    Skill,                          // 스킬 보상(해금)
    WeaponUpgrade,                  // 무기 강화 보상
    StatUpgrade,                    // 스텟 강화 보상
};

enum StatCode : uint8
{
    Health_S = 1,                   // 체력 스텟
    Speed_S,                        // 이동 속도 스텟
};

struct StoreEntryDef
{
    uint32 entryId{0};
    int32 cost{0};
    
    StoreRewardType rewardType{StoreRewardType::None};
    
    ItemId itemId{0};               // 아이템이라면...
    int32 itemCount{0};             // 아이템 보상 개수
    
    SkillId skillId{0};              // 스킬이라면...
    
    uint32 weaponUpgradeType{0};    // 무기 강화라면... (강화할 종류)
    
    uint32 statUpgradeType{0};      // 스텟 강화라면... (강화할 종류)
    int32 statUpgradeMaxLevel{0};     // 스텟 강화라면... (강화 가능한 최대 Level)
};

struct StoreBuyResult
{
    bool success{false};
    StoreBuyResultCode resultCode{StoreBuyResultCode::ApplyFailed};
    
    ObjectId playerId;
    ObjectId storeId;
    uint32 entryId;
};

struct StoreBuyContext
{
    PlayerPawn* playerPawn{nullptr};
    Actor* storeActor{nullptr};
    
    const StoreEntryDef* entryDef{nullptr};
    uint32 entryId{0};
};

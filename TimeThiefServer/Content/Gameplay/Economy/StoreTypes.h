#pragma once
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
    PlayerPawn* playerPawn;
    Actor* storeActor;
    uint32 entryId;
    
    int64 cost;
};

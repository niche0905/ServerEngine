#include "pch.h"
#include "ChestActor.h"
#include "PlayerPawn.h"
#include "Service/Room/Room.h"

/*--------------
   ChestActor
--------------*/

LootBundle ChestActor::GenerateDrops()
{
    if (isOpened_) {
        // 이미 열린 상자에서 드롭을 생성하려는 경우, 빈 LootBundle 반환
        return LootBundle{};
    }
    
    auto room = GetRoom();
    if (room == nullptr)
        return LootBundle{};
    
    isOpened_ = true;
    return room->GetRoomGameSystem().GetLootSystem().GenerateLootBundle(lootSource_.GetTableId());
}

void ChestActor::OnSpawn()
{
    StaticActor::OnSpawn();
    
    lootSource_.Init(this, 1);       // TEMP: LootSourceComponent의 tableId는 1로 고정
}

void ChestActor::OnPreDestroy()
{
    StaticActor::OnPreDestroy();
}

bool ChestActor::CheckOpenPermission(PlayerPawn& byPlayer, int32& outError) const
{
    if (isOpened_) {
        outError = 1; // 이미 열린 상자
        return false;
    }
    
    const auto& pos = byPlayer.GetPosition();
    const auto& chestPos = GetPosition();
    const float distSq = (pos - chestPos).LengthSq();
    
    const float kDist = 300.f;  // 상호작용 허용 최대 거리 (3m)
                                // TODO: Config 값으로 빼기
    if (distSq > kDist * kDist) {
        outError = 2;   // 플레이어가 상자에서 너무 멀리 떨어져 있음
        return false;
    }
    
    // 오픈에 성공
    outError = 0;
    return true;
}

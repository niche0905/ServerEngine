#include "pch.h"
#include "ChestActor.h"
#include "PlayerPawn.h"
#include "Service/Room/Room.h"

/*--------------
   ChestActor
--------------*/

LootBundle ChestActor::GenerateDrops()
{
    auto room = GetRoom();
    if (room == nullptr)
        return LootBundle{};
    
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

bool ChestActor::CheckOpenPermission(ObjectManager& om, PlayerPawn& byPlayer, int32& outError) const
{
    (void)om;
    
    // TODO: 거리 체크 / 시야 체크 / 상태 체크
    (void)byPlayer;
    
    // 오픈에 성공
    outError = 0;
    return true;
}

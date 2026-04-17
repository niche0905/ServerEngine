#include "pch.h"
#include "ChestActor.h"
#include "PlayerPawn.h"

/*--------------
   ChestActor
--------------*/

LootBundle ChestActor::GenerateDrops()
{
    // TODO: Loot 이용해서 처리 해도 되고
    //       Init 시점에서 미리 아이템을 채워 놓는 방식으로 해도 될 듯
    
     return LootBundle{};
}

void ChestActor::OnSpawn()
{
    StaticActor::OnSpawn();
    
    lootSource_.Init(GetId(), 0);
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

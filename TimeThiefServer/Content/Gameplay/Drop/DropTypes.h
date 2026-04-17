#pragma once
#include "Content/Gameplay/Loot/LootTypes.h"
#include "Content/Object/ObjectId.h"

enum class DropMode : uint8
{
    CorpseBox,  // 시체 상자 (배틀그라운드나 에이펙스 레전드 차용)
    Scatter,    // 흩뿌리기 (디아블로, 포트나이트 차용)
};

enum class DropReason : uint8
{
    Unknown = 0,        // 알 수 없음
    
    Chest,
    Death,  // Player Death, Monster Death
    
    Manual,
    Script,
};

struct DropSpawnContext
{
    DropMode        mode{DropMode::Scatter};            // 드롭 생성 방식 (CorpseBox, Scatter)
    DropReason      reason{DropReason::Unknown};        // 드롭 생성 이유 (Pawn의 사망, Chest의 Open)
    ObjectId        owner{};                            // 드롭을 생성하는 오브젝트 (죽은 플레이어/몬스터/상자)
    ObjectId        instigator{};                       // 킬러/원인
    LootBundle      lootBundle;                         // 드롭할 아이템 정보 (Pawn, Chest가 주체적으로 판단하여 생성)
};

struct DropSpawnResult
{
    bool spawned{false};
    int32 spawnedCount{0};      // 생성된 드롭 아이템의 개수
};

struct SpawnWorldItemParams
{
    ItemStack itemStack{};             // 생성할 아이템의 정보 (템플릿 ID, 수량)
    SE::Math::Vector3 position{};
    SE::Math::Vector3 initialVelocity{};
    DropReason reason{DropReason::Unknown};
};

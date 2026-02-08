#pragma once
#include "Content/Gameplay/Drop/DropTypes.h"
#include "Content/Gameplay/Loot/LootTypes.h"
#include "Content/Object/ObjectId.h"

class ObjectManager;
struct SE::Math::Vector3;

struct DropSpawnContext
{
    ObjectId owner{};       // 드롭을 생성하는 오브젝트 (죽은 플레이어/몬스터)
    ObjectId instigator{};  // 킬러/원인
    uint64 nowMs{0};
};

struct DropSpawnResult
{
    bool spawned{false};
    ObjectId spawnedActor{};    // 생성된 드롭 액터의 ID, CorpseBox면 해당 actor id, Scatter라면 대표 값이 없을 수 있음
    int32 spawnedCount{0};      // 생성된 드롭 아이템의 개수
};

/*---------------------
   DropSpawnService
---------------------*/
//
// DropSpawnService
//

class DropSpawnService
{
public:
    using Vector3 = SE::Math::Vector3;
    
    // TODO: 각자 필요한 Actor들 만들고 구현체 작성하기
public:
    DropSpawnResult Spawn(ObjectManager& om, const LootBundle& bundle, const DropSpawnPolicy& policy, const Vector3& position, const DropSpawnContext& ctx);
    
private:
    DropSpawnResult SpawnCorpseBox(ObjectManager& om, const LootBundle& bundle, const Vector3& position, const DropSpawnContext& ctx);
    DropSpawnResult SpawnScatter(ObjectManager& om, const LootBundle& bundle, const DropSpawnPolicy& policy, const Vector3& position, const DropSpawnContext& ctx);
    
};

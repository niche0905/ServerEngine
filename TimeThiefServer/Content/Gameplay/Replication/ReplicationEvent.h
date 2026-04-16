#pragma once
#include "Content/Object/ObjectId.h"
#include "ReplicationTypes.h"
#include <vector>
#include <common/common_types.pb.h>

#include "Physics/Narrowphase/IntersectUtil.h"

enum class RepEventType : uint8
{
    None = 0,
    
    Spawn,
    Despawn,
    
    // Damage,
    // Death,
    // Respawn,
    //
    // Fire,
    // ProjectileExplode,
    //
    // ItemUseStart,
    // ItemUseCancel,
    // ItemUseComplete,
    //
    // AttackStart,
    // AttackHit,
};

struct RepEvent
{
    RepEventType type{RepEventType::None};
    ObjectId source{};
    TickSeq tick{0};
    uint64 timeMs{0};
};

struct RepFireEvent
{
    ObjectId source{};
    uint32 weaponId{0};
    uint32 shotSeed{0};
    SE::Math::Vector3 origin{};
    SE::Math::Vector3 direction{};
    TickSeq tick{0};
    uint64 timeMs{0};
};

struct RepThrowEvent
{
    ObjectId source{};
    uint32 grenadeType{0};
    SE::Math::Vector3 origin{};
    SE::Math::Vector3 direction{};
    TickSeq tick{0};
    uint64 timeMs{0};
};

struct RepReloadEvent
{
    ObjectId source{};
    uint32 weaponId{0};
    TickSeq tick{0};
    uint64 timeMs{0};
};

struct RepHitEvent
{
    ObjectId source{};
    se::common::Vector3 hitPoint{};
    TickSeq tick{0};
    uint64 timeMs{0};
};

// struct RepUseAbilityEvent
// {
//     ObjectId source{};
//     uint32 abilityId{0};
//     TickSeq tick{0};
//     uint64 timeMs{0};
// };

// struct RepUseItemEvent
// {
//     ObjectId source{};
//     uint32 itemId{0};
//     TickSeq tick{0};
//     uint64 timeMs{0};
// };

// struct RepChestEvent
// {
//     ObjectId source{};
//     ObjectId chestId{0};
//     TickSeq tick{0};
//     uint64 timeMs{0};
// };

// struct RepPickupEvent
// {
//     ObjectId source{};
//     ObjectId itemId{0};
//     TickSeq tick{0};
//     uint64 timeMs{0};
// };

struct RepWireLaunchEvent
{
    ObjectId source{};
    SE::Math::Vector3 origin{};
    SE::Math::Vector3 direction{};
    TickSeq tick{0};
    uint64 timeMs{0};
};

struct RepWireEvent
{
    ObjectId source{};
    SE::Math::Vector3 anchorPos{};
    TickSeq tick{0};
    uint64 timeMs{0};
};

struct RepWireEndEvent
{
    ObjectId source{};
    TickSeq tick{0};
    uint64 timeMs{0};
};

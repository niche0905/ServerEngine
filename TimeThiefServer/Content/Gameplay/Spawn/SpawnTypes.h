#pragma once
#include "Content/Object/ObjectId.h"

enum class SpawnKind : uint8
{
    None = 0,
    
    Monster,
    NPC,
    Prop,
    LootBox,
};

enum class SpawnMode : uint8
{
    FixedPoint = 0,     // 고정 위치 스폰
    Radius,             // 반경 내 랜덤 위치 스폰 
};

enum class DespawnReason : uint8
{
    Unknown = 0,
    
    LifetimeExpired,
    // TooFarFromPlayers,
    Script,
    WaveReset,
    RoomReset,
};

struct SpawnPoint
{
    using Vector3 = SE::Math::Vector3;
    
    int32 id{0};
    SpawnKind kind{SpawnKind::None};
    
    int32 templateId{0};
    
    Vector3 position{};
    
    SpawnMode mode{SpawnMode::FixedPoint};
    float radius{0.0f};             // SpawnMode가 Radius일 때 유효
    
    uint32 respawnDelayMs{0};       // 리스폰 지연 시간 (밀리초), 0이면 리스폰 안 함
    
    uint32 lifetimeMs{0};           // 생존 시간 (밀리초), 0이면 무한
    // float despawnDistance{0.0f};    // (미사용) 플레이어로부터 멀어지면 소멸 거리, 0이면 비활성화
};

struct SpawnRequest
{
    int32 spawnPointId{0};
    int32 templateId{0};
    SpawnKind kind{SpawnKind::None};
    
    // override가 필요한 경우
    uint32 lifetimeMs{0};
    // float despawnDistance{0.0f};
};

struct SpawnResult
{
    bool spawned{false};
    ObjectId object{};
    int32 spawnPointId{0};
};

struct DespawnRequest
{
    ObjectId object{};
    DespawnReason reason{DespawnReason::Unknown};
    uint64 nowMs{0};
};

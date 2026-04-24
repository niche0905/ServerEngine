#pragma once
#include <Generated/ServerPacketHandler.h>

class Room;
struct SE::Math::Vector3;
using TickSeq = uint32;

struct RepFrame
{
    TickSeq roomTick{0};    // Room 로컬 tick 번호
    TimePoint now{};        // flush 시점의 절대 시간
    Milliseconds dt{0}; // 이번 Room Tick에 사용된 dt (기본 50ms)
};

enum class ReplicationDirty : uint32
{
    None            = 0,
    Transform       = 1 << 0,
    Velocity        = 1 << 1,
    Health          = 1 << 2,   // Player Only  (체력 상태)
    Resource        = 1 << 3,   // Player Only  (자원 상태 <- 재화 한정)
    Inventory       = 1 << 4,   // Player Only  (인벤토리 아이템 상태)
    AnimState       = 1 << 5,   // (애니메이션 상태) <- NPC, Player 범용
    CombatState     = 1 << 6,   
    SkillState      = 1 << 7,   // Player Only  (스킬 해금 상태)
    WeaponStat      = 1 << 8,   // Player Only  (무기 강화 상태)
};

struct ReplicateResult
{
    bool sent = false;
    ReplicationDirty handled = ReplicationDirty::None;   // 실제로 패킷이 생성되어 전송된 정보 (ReplicationDirty 플래그)
};

inline ReplicationDirty operator|(ReplicationDirty a, ReplicationDirty b)
{
    return static_cast<ReplicationDirty>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline ReplicationDirty& operator|=(ReplicationDirty& a, ReplicationDirty b)
{
    a = a | b;
    return a;
}

inline bool HasDirty(ReplicationDirty value, ReplicationDirty flag)
{
    return (static_cast<uint32>(value) & static_cast<uint32>(flag)) != 0;
}

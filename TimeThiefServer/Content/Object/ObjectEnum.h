#pragma once

/*--------------
   EntityType
--------------*/
//
// EntityType는 게임 내에서 다양한 유형의 엔티티를 구분하기 위한 열거형입니다.
// 아마.. GameShared로 위임 될 가능성이 다수 존재한다...
//

enum class EntityType : uint8
{
    None = 0,
    
    Player,
    NPC,
    Item,
    Projectile,
};

/*----------------
   ObjectState
----------------*/
//
// ObjectState는 오브젝트의 현재 상태를 나타냅니다.
//

enum class ObjectState : uint8
{
    Alive               = 0,
    PendingDestroy,
    Destroyed
};

/*----------------
   ObjectFlags
----------------*/
//
// ObjectFlags는 오브젝트의 특성을 나타내는 플래그입니다.
//

enum class ObjectFlags : uint32
{
    None            = 0,
    Tickable        = 1 << 0,   // 서버 틱에서 매 프레임 업데이트 되는 오브젝트
    Replicable      = 1 << 1,   // 네트워크로 복제되는 오브젝트
    Spatialized     = 1 << 2,   // 공간화된 오브젝트
};


inline ObjectFlags operator|(ObjectFlags a, ObjectFlags b)
{
    return static_cast<ObjectFlags>(static_cast<uint32>(a) | static_cast<uint32>(b));
}

inline bool HasFlag(ObjectFlags flags, ObjectFlags flagToCheck)
{
    return (static_cast<uint32>(flags) & static_cast<uint32>(flagToCheck)) != 0;
}

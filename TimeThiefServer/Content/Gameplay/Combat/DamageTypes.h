#pragma once

struct ObjectId;

enum class DamageType : int8
{
    None = 0,
    
    Melee,
    Ranged,
    Explosion,
    Dot,
    TrueDamage,
};

enum class DamageSource : int8
{
    Unknown = 0,
    
    Skill,
    Weapon,
    Environment,
};

struct DamageContext
{
    ObjectId attacker{};    // 공격자
    ObjectId instigator{};  // 발사체 등의 주인
    int32 skillId{0};       // TODO: 나중에 스킬 시스템으로 확장
    DamageType type{DamageType::None};
    DamageSource source{DamageSource::Unknown};
    
    // 필요 시 확장
};

struct DamageResult
{
    int32 requested{0};
    int32 applied{0};
    int32 hpBefore{0};
    int32 hpAfter{0};
    bool killed{false};
    bool accepted{false};
};

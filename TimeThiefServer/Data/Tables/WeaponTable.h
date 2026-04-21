#pragma once
#include <variant>

enum class WeaponFireType: uint8
{
    HitScan,            // 즉발 판정 (라이플, 샷건)
    Projectile,         // 투사체 발사 (로켓 런처)
};

enum class WeaponCategory : uint8
{
    None = 0,
    
    Rifle,
    Shotgun,
    Launcher,
};

struct WeaponCommonStat
{
    WeaponCategory              category = WeaponCategory::None;
    WeaponFireType              fireType = WeaponFireType::HitScan;
    
    int                         damage = 0;
    int                         magCapacity = 0;
    float                       fireIntervalSec = 0.0f;
    float                       reloadTimeSec = 0.0f;
    float                       range = 0.0f;
};

struct RifleStat
{
    
};

struct ShotgunStat
{
    int                         pelletCount = 0;
    float                       coneAngleDegrees = 0.0f;
};

struct LauncherStat
{
    float                       projectileSpeed = 0.0f;
    float                       explosionRadius = 0.0f;
};

using WeaponExtraStat = std::variant<RifleStat, ShotgunStat, LauncherStat>;

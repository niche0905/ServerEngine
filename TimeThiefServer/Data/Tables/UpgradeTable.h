#pragma once
#include "WeaponTable.h"

enum class WeaponUpgradeTargetType : uint8
{
    AllWeapons,
    Category,
    WeaponId
};

struct WeaponUpgradeTarget
{
    // UpgradeTargetType       type = UpgradeTargetType::AllWeapons;
    // WeaponCategory          category = WeaponCategory::None;
    uint32                  weaponId = 0;
};

struct WeaponStatModifier
{
    int                     damageDelta = 0;
    int                     magCapacityDelta = 0;
    float                   fireIntervalSecDelta = 0.0f;
    float                   reloadTimeSecDelta = 0.0f;
    float                   rangeDelta = 0.0f;
    
    int                     palletCountDelta = 0;
    float                   coneAngleDegreesDelta = 0.0f;
    
    float                   projectileSpeedDelta = 0.0f;
    float                   explosionRadiusDelta = 0.0f;
};

struct WeaponUpgradeDef
{
    WeaponUpgradeTarget     target{};
    WeaponStatModifier      modifier{};
};

struct WeaponUpgradeTable
{
    std::unordered_map<WeaponUpgradeCode, WeaponUpgradeDef> defs;
    
    const WeaponUpgradeDef* Find(WeaponUpgradeCode code) const
    {
        auto it = defs.find(code);
        return (it != defs.end()) ? &it->second : nullptr;
    }
};

struct StatUpgradeDef
{
    uint32              statCode = 0;
    int32               statDelta = 0;
};

struct StatUpgradeEntry
{
    std::vector<StatUpgradeDef> statUpgrades;
    
    const StatUpgradeDef* GetLevelByStat(int32 level) const
    {
        if (statUpgrades.empty()) return nullptr;
        if (level >= static_cast<int>(statUpgrades.size()) or level < 0) return nullptr;
        
        return &statUpgrades[level];
    }
};

struct StatUpgradeTable
{
    std::unordered_map<StatUpgradeCode, StatUpgradeEntry> statUpgrades;
    
    const StatUpgradeEntry* Find(StatUpgradeCode code) const
    {
        auto it = statUpgrades.find(code);
        return (it != statUpgrades.end()) ? &it->second : nullptr;
    }
};

struct UpgradeTable
{
    WeaponUpgradeTable      WeaponUpgradeTable;
    StatUpgradeTable        StatUpgradeTable;
};

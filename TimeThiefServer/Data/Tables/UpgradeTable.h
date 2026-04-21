#pragma once
#include "WeaponTable.h"

enum class UpgradeTargetType : uint8
{
    AllWeapons,
    Category,
    WeaponId
};

struct UpgradeTarget
{
    UpgradeTargetType       type = UpgradeTargetType::AllWeapons;
    WeaponCategory          category = WeaponCategory::None;
    uint32                  weaponId = 0;
};

struct WeaponStatModifier
{
    int                     damageDelta = 0;
    int                     magCapacityDelta = 0;
    float                   fireIntervalSecDelta = 0.0f;
    float                   reloadTimeDelta = 0.0f;
    float                   rangeDelta = 0.0f;
    
    int                     palletCountDelta = 0;
    float                   coneAngleDegreesDelta = 0.0f;
    
    float                   projectileSpeedDelta = 0.0f;
    float                   explosionRadiusDelta = 0.0f;
};

struct WeaponUpgradeDef
{
    WeaponUpgradeCode       upgradeCode = 0;
    UpgradeTarget           target{};
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

struct UpgradeTable
{
    WeaponUpgradeTable WeaponUpgradeTable;
};

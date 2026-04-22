#pragma once
#include "Content/Gameplay/Inventory/ItemTypes.h"

struct RespawnSnapshot
{
    SE::Math::Vector3 position{};             // 세이브 포인트 위치 (RespawnComp를 사용할 것이기에 참고만)
};

struct HealthSnapshot
{
    int32 health = 0;             // 플레이어 체력
};

struct InventorySnapshot
{
    std::vector<ItemStack> inventoryItems;         // 플레이어 인벤토리에 있는 아이템들
};

struct SkillSnapshot
{
    std::unordered_set<SkillId> unlockSkills;   // 플레이어가 잠금 해제한 스킬 ID 집합
    std::array<SkillId, kMaxActiveSkills> equippedSkills{};       // 플레이어가 장착한 스킬 ID 배열
};

struct UpgradeSnapshot
{
    std::unordered_set<WeaponUpgradeCode> weaponUpgradeCodes;    // 플레이어가 획득한 무기 업그레이드 코드들의 집합
    std::unordered_map<StatUpgradeCode, int32> statUpgradeLevels;     // 플레이어가 획득한 스탯 업그레이드 코드와 해당 레벨의 맵
};


struct SavePointSnapshot
{
    // Respawn
    RespawnSnapshot         respawnSnapshot;
    
    // Stat
    HealthSnapshot          healthSnapshot;
    
    // Inventory
    InventorySnapshot       inventorySnapshot;
    
    // Skill
    SkillSnapshot           skillSnapshot;

    // Upgrade
    UpgradeSnapshot         upgradeSnapshot;
};

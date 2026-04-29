#pragma once
#include "TypesDef.h"
#include "Content/Shared/BaseComponent.h"

/*-------------------
   SkillComponent
-------------------*/
//
// SkillComponent는 플레이어가 사용할 수 있는 스킬을 관리하는 컴포넌트입니다.
//

class SkillComponent : public BaseComponent
{
public:
    
public:
    virtual void Init(BaseObject* owner);
   
public:
    bool HasSkill(SkillId skillId) const;
    bool CanUnlockSkill(SkillId skillId) const;
    bool UnlockSkill(SkillId skillId);
    
    bool IsEquipped(SkillId skillId) const;
    SkillId GetEquippedSkill(int32 slotIndex) const;
    
    bool CanEquipSkill(SkillId skillId, int32 slotIndex) const;
    bool EquipSkill(SkillId skillId, int32 slotIndex);
    bool UnequipSkill(int32 slotIndex);
    
    const std::unordered_set<SkillId>& GetUnlockSkills() const;
    
private:
    bool IsValidSlot(int32 slotIndex) const;
    
private:
    std::unordered_set<SkillId> unlockSkills_;              // 플레이어가 잠금 해제한 스킬 ID 집합
    std::array<SkillId, MaxActiveSkills> activeSkills_;    // 현재 장착된 스킬 ID 배열
    
};

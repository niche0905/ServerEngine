#include "pch.h"
#include "SkillComponent.h"
#include "Content/Object/Actor/PlayerPawn.h"

/*-------------------
   SkillComponent
-------------------*/

void SkillComponent::Init(BaseObject* owner)
{
   SetOwner(owner);
   
   for (SkillId& skillId : activeSkills_) {
      skillId = 0;   // 0은 스킬이 장착되지 않은 상태를 나타냄
   }
}

bool SkillComponent::HasSkill(SkillId skillId) const
{
   if (skillId == 0) {
      return false;   // 0은 유효한 스킬 ID가 아님
   }
   
   return unlockSkills_.contains(skillId);
}

bool SkillComponent::CanUnlockSkill(SkillId skillId) const
{
   if (skillId == 0) {
      return false;   // 0은 유효한 스킬 ID가 아님
   }
   
   return !HasSkill(skillId);
}

bool SkillComponent::UnlockSkill(SkillId skillId)
{
   if (!CanUnlockSkill(skillId)) {
      return false;
   }
   
   unlockSkills_.insert(skillId);
   return true;
}

bool SkillComponent::IsEquipped(SkillId skillId) const
{
   if (skillId == 0) {
      return false;   // 0은 유효한 스킬 ID가 아님
   }
   
   for (const SkillId& equippedId : activeSkills_) {
      if (equippedId == skillId) {
         return true;
      }
   }
   
   return false;
}

SkillId SkillComponent::GetEquippedSkill(int32 slotIndex) const
{
   if (!IsValidSlot(slotIndex)) {
      return 0;   // 유효하지 않은 슬롯 인덱스
   }
   
   return activeSkills_[slotIndex];
}

bool SkillComponent::CanEquipSkill(SkillId skillId, int32 slotIndex) const
{
   if (skillId == 0) {
      return false;   // 0은 유효한 스킬 ID가 아님
   }
   
   if (!IsValidSlot(slotIndex)) {
      return false;
   }
   
   if (!HasSkill(skillId)) {
      return false;   // 스킬이 잠금 해제되지 않음
   }
   
   for (int32 i = 0; i < kMaxActiveSkills; ++i) {
      if (i == slotIndex) {
         continue;   // 현재 슬롯은 검사에서 제외
      }
      
      if (activeSkills_[i] == skillId) {
         return false;   // 이미 다른 슬롯에 장착된 스킬
      }
   }
   
   return true;
}

bool SkillComponent::EquipSkill(SkillId skillId, int32 slotIndex)
{
   if (!CanEquipSkill(skillId, slotIndex)) {
      return false;
   }
   
   activeSkills_[slotIndex] = skillId;
   return true;
}

bool SkillComponent::UnequipSkill(int32 slotIndex)
{
   if (!IsValidSlot(slotIndex)) {
      return false;
   }
   
   activeSkills_[slotIndex] = 0;   // 슬롯을 비움
   return true;
}

bool SkillComponent::IsValidSlot(int32 slotIndex) const
{
   return slotIndex >= 0 && slotIndex < kMaxActiveSkills;
}

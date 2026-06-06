#include "pch.h"
#include "SkillComponent.h"
#include "Content/Object/Actor/PlayerPawn.h"

/*-------------------
   SkillComponent
-------------------*/

void SkillComponent::Init(BaseObject* owner)
{
   SetOwner(owner);
   
   unlockSkills_.clear();
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

SkillComponent::SkillUnlockResult SkillComponent::TryUnlockSkill(SkillId skillId)
{
   SkillUnlockResult result;
   
   if (!CanUnlockSkill(skillId)) {
      return result;
   }
   
   unlockSkills_.insert(skillId);
   result.unlocked = true;
   result.autoEquipped = TryAutoEquipSkill(skillId, &result.equippedSlotIndex);
   
   if (!result.autoEquipped) {
      if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
         player->OnSkillChanged(skillId);
      }
   }
   
   return result;
}

bool SkillComponent::UnlockSkill(SkillId skillId)
{
   return TryUnlockSkill(skillId).unlocked;
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

int32 SkillComponent::FindEmptySlot() const
{
   for (int32 i = 0; i < MaxActiveSkills; ++i) {
      if (activeSkills_[i] == 0) {
         return i;
      }
   }
   
   return -1;
}

SkillId SkillComponent::GetEquippedSkill(int32 slotIndex) const
{
   if (!IsValidSlot(slotIndex)) {
      return 0;   // 유효하지 않은 슬롯 인덱스
   }
   
   return activeSkills_[slotIndex];
}

const std::array<SkillId, MaxActiveSkills>& SkillComponent::GetEquippedSkills() const
{
   return activeSkills_;
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
   
   for (int32 i = 0; i < MaxActiveSkills; ++i) {
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
   
   if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
      player->OnSkillChanged(skillId);
   }
   
   return true;
}

bool SkillComponent::TryAutoEquipSkill(SkillId skillId, int32* outSlotIndex)
{
   if (outSlotIndex) {
      *outSlotIndex = -1;
   }
   
   const int32 emptySlot = FindEmptySlot();
   if (emptySlot < 0) {
      return false;
   }
   
   if (!EquipSkill(skillId, emptySlot)) {
      return false;
   }
   
   if (outSlotIndex) {
      *outSlotIndex = emptySlot;
   }
   
   return true;
}

bool SkillComponent::UnequipSkill(int32 slotIndex)
{
   if (!IsValidSlot(slotIndex)) {
      return false;
   }
   
   const SkillId prevSkillId = activeSkills_[slotIndex];
   
   activeSkills_[slotIndex] = 0;   // 슬롯을 비움
   
   if (prevSkillId != 0) {
      if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
         player->OnSkillChanged(prevSkillId);
      }
   }
   
   return true;
}

const std::unordered_set<SkillId>& SkillComponent::GetUnlockSkills() const
{
   return unlockSkills_;
}

SkillSnapshot SkillComponent::CaptureSnapshot() const
{
   SkillSnapshot result;
   result.unlockSkills = unlockSkills_;
   for (int32 i = 0; i < MaxActiveSkills; ++i){
      result.equippedSkills[i] = activeSkills_[i];
   }
   return result;
}

void SkillComponent::RestoreSnapshot(const SkillSnapshot& snapshot)
{
   unlockSkills_ = snapshot.unlockSkills;
   for (int32 i = 0; i < MaxActiveSkills; ++i) {
      activeSkills_[i] = snapshot.equippedSkills[i];
   }

   if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
      player->MarkReplicationDirty(ReplicationDirty::SkillState);
   }
}

bool SkillComponent::IsValidSlot(int32 slotIndex) const
{
   return slotIndex >= 0 && slotIndex < MaxActiveSkills;
}

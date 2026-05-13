#include "pch.h"
#include "SaveComponent.h"
#include "Content/Object/Actor/PlayerPawn.h"

/*-----------------
   SaveComponent
-----------------*/

void SaveComponent::Init(BaseObject* owner)
{
   SetOwner(owner);
}

bool SaveComponent::CaptureSnapshot()
{
   PlayerPawn* player = GetOwnerAs<PlayerPawn>();
   if (!player)
      return false;   // PlayerPawn이 아닌 경우, Snapshot을 캡처할 수 없음
   
   snapshot_.respawnSnapshot.position = player->GetSavedRespawnPosition();
   
   snapshot_.healthSnapshot.health = player->GetHp();
   
   snapshot_.inventorySnapshot = player->GetInventory().CaptureSnapshot();
   snapshot_.skillSnapshot = player->GetSkill().CaptureSnapshot();
   snapshot_.upgradeSnapshot = player->GetUpgrade().CaptureSnapshot();
   
   return true;
}

bool SaveComponent::Rollback()
{
   PlayerPawn* player = GetOwnerAs<PlayerPawn>();
   if (!player)
      return false;   // PlayerPawn이 아닌 경우, Rollback을 수행할 수 없음
   
   player->GetUpgrade().RestoreSnapshot(snapshot_.upgradeSnapshot);
   player->GetSkill().RestoreSnapshot(snapshot_.skillSnapshot);
   player->GetInventory().RestoreSnapshot(snapshot_.inventorySnapshot);
   
   player->GetHealth().Revive(snapshot_.healthSnapshot.health);
   
   return false;
}

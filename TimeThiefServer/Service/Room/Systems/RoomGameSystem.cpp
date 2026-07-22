#include "pch.h"
#include "RoomGameSystem.h"
#include "Content/Object/Actor/Pawn.h"
#include "Content/Object/ObjectId.h"
#include "Data/GameDataManager.h"
#include "Network/ServerConfig.h"
#include "Service/Room/Room.h"

/*------------------
   RoomGameSystem
------------------*/

bool RoomGameSystem::Init(Room* ownerRoom, const GameDataManager& gameDataManager, const GameConfig& gameConfig)
{
   if (ownerRoom == nullptr)
      return false;
   
   ownerRoom_ = ownerRoom;
   gameDataManager_ = &gameDataManager;
   
   // TEMP: 아래 ZoneBounds 값은 나중에 제대로 바꾸어야 함 (Map Data를 읽던, 하드 코딩으로 Map min, max를 넣던)
   if (!zoneSystem_.Init(ownerRoom, ZoneBounds{ SE::Math::Vector3{}, SE::Math::Vector3{110000.0, 110000.0, 0.0f} }, gameDataManager_->GetZoneTable(), gameConfig.zoneDamageTickInterval))
      return false;
   
   if (!respawnSystem_.Init(ownerRoom))
      return false;
   
   if (!replicationSystem_.Init(ownerRoom))
      return false;
   
   if (!dropSystem_.Init(ownerRoom))
      return false;
   
   if (!lootSystem_.Init(ownerRoom, gameDataManager_->GetLootTable()))
      return false;
   
   if (!storeSystem_.Init(ownerRoom, gameDataManager_->GetStoreEntryTable()))
      return false;
   
   if (!weaponSystem_.Init(ownerRoom, gameDataManager_->GetWeaponTable(), gameDataManager_->GetUpgradeTable()))
      return false;
   
   if (!upgradeSystem_.Init(ownerRoom, gameDataManager_->GetUpgradeTable()))
      return false;
   
   if (!combatSystem_.Init(ownerRoom, gameDataManager_->GetServerMap()))
      return false;
   
   return true;
}

bool RoomGameSystem::Start()
{
   if (!zoneSystem_.Start())
      return false;
   
   return true;
}

void RoomGameSystem::Update(float deltaTime)
{
   zoneSystem_.Update(deltaTime);
}

void RoomGameSystem::OnPawnDeath(ObjectId pawnId)
{
   if (not respawnSystem_.RequestRespawn(pawnId)) {
      Pawn* pawn = ownerRoom_->GetObjectManager().FindAs<Pawn>(pawnId);
      if (pawn && pawn->IsPlayer()) {
         ownerRoom_->OnRealDeath(pawnId);
      }
      else {
         ownerRoom_->HandleDespawn(pawnId);
      }
   }
}

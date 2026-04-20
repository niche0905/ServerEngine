#pragma once
#include "Content/Gameplay/Economy/StoreTypes.h"
#include "Data/Tables/StoreEntryTable.h"

class Room;

/*----------------
   StoreSystem
----------------*/
//
// StoreSystem는 플레이어가 게임 내에서 아이템을 구매할 수 있도록 하는 시스템입니다.
//

class StoreSystem
{
public:
   StoreSystem() = default;
   
   bool Init(Room* ownerRoom, const StoreEntryTable& storeEntryTable);
   
   StoreBuyResult Buy(const StoreBuyRequest& req);
   
private:
   StoreBuyResult ValidateBuyRequest(const StoreBuyRequest& req, StoreBuyContext& ctx);
   bool TryConsumeCost(StoreBuyContext& ctx);
   bool TryApplyReward(StoreBuyContext& ctx);
   
   const StoreEntryDef* FindStoreEntry(uint32 entryId) const;
   bool CanInteractStore(PlayerPawn* playerPawn, Actor* storeActor) const;
   bool CanBuy(PlayerPawn* playerPawn) const;
   
private:
   bool CanPurchaseReward(PlayerPawn* playerPawn, const StoreEntryDef* entryDef) const;
   
   bool TryApplyItemReward(const StoreBuyContext& ctx);
   bool TryApplySkillReward(const StoreBuyContext& ctx);
   bool TryApplyWeaponUpgradeReward(const StoreBuyContext& ctx);
   bool TryApplyStatUpgradeReward(const StoreBuyContext& ctx);
   
private:
   Room*                   ownerRoom_ = nullptr;   // non-owning
   const StoreEntryTable*  storeEntryTable_ = nullptr;
    
};

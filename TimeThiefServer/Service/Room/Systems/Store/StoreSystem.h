#pragma once
#include "Content/Gameplay/Economy/StoreTypes.h"

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
   
   bool Init(Room* ownerRoom);
   
   StoreBuyResult Buy(const StoreBuyRequest& req);
   
private:
   StoreBuyResult ValidateBuyRequest(const StoreBuyRequest& req, StoreBuyContext& ctx);
   bool TryConsumeCost(StoreBuyContext& ctx);
   bool TryApplyReward(StoreBuyContext& ctx);
   
   int64 FindStoreEntry(uint32 entryId) const;
   bool CanInteractStore(PlayerPawn* playerPawn, Actor* storeActor) const;
   bool CanBuy(PlayerPawn* playerPawn) const;
   
private:
   Room* ownerRoom_ = nullptr;   // non-owning
    
};

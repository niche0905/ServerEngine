#pragma once
#include "Content/Gameplay/Inventory/ItemTypes.h"
#include "Content/Gameplay/Economy/WalletTypes.h"
#include "Content/Gameplay/Loot/LootTypes.h"

class ObjectManager;
class InvectoryComponent;
class WalletComponent;

/*---------------------
   IDropOnDeathOwner
---------------------*/
//
// IDropOnDeathOwner는 죽을 때 아이템이나 화폐를 드롭할 수 있는 객체가 구현해야 하는 인터페이스입니다.
//

class IDropOnDeathOwner
{
public:
   virtual ~IDropOnDeathOwner() = default;
   
   virtual InvectoryComponent& GetInventory() = 0;
   virtual WalletComponent& GetWallet() = 0;
   
   virtual bool IsConsumable(ItemId itemId) const = 0;
   
   virtual bool CanDropOnDeath() const { return true; }
   
};

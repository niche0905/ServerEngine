#pragma once
#include "Content/Shared/BaseComponent.h"

/*-----------------------
   LootSourceComponent
-----------------------*/
//
// LootSourceComponent는 게임 내에서 아이템을 생성할 수 있는 오브젝트에 부착되는 컴포넌트입니다.
// Loot Table을 참조하여 아이템 드랍에 대한 정보를 관리합니다
//

class LootSourceComponent : public BaseComponent
{
public:
    void Init(ObjectId owner, int32 tableId)
    {
        SetOwner(owner);
      
        tableId_ = tableId;
    }
    
    int32 GetTableId() const { return tableId_; }
   
private:
    int32 tableId_{0};
   
};

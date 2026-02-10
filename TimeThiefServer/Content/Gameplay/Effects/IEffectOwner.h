#pragma once
#include "Content/Object/ObjectId.h"

class ObjectManager;
class HealthComponent;
class WalletComponent;
class InventoryComponent;
class EffectComponent;

/*----------------
   IEffectOwner
----------------*/
//
// IEffectOwner는 효과(버프/디버프 등)를 소유하는 객체를 나타내는 인터페이스입니다.
//

class IEffectOwner
{
public:
    virtual ~IEffectOwner() = default;
    
    virtual EffectComponent& GetEffectComponent() = 0;
    
    virtual HealthComponent* TryGetHealth() = 0;        // 체력 컴포넌트 (있을 수도 있고 없을 수도 있음)
    
    // MEMO: 필요해지면 점진적으로 추가
    
    virtual void OnEffectChanged() {}
    
};

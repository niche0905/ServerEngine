#pragma once
#include "Content/Shared/BaseComponent.h"
#include "CombatTypes.h"

class Pawn;

/*-------------------
   CombatComponent
-------------------*/
//
// CombatComponent는 Pawn의 전투 관련 기능을 담당하는 컴포넌트입니다.
// 상속 받아 Player와 NPC로 나뉘어 구현될 것을 상정하였습니다.
//

class CombatComponent : public BaseComponent
{
public:
   virtual ~CombatComponent() = default;
   
   virtual void Init(BaseObject* owner);
   
   void SetOwnerPawn(Pawn* pawn);
   Pawn* GetOwnerPawn() const;
   
   virtual bool CanAttack(const AttackRequest& request) const;
   virtual bool TryAttack(AttackRequest& request);
   
protected:
   virtual bool ExecuteAttack(AttackRequest& request) = 0;
   
private:
   Pawn* ownerPawn_ = nullptr;   // non-owning cache
    
};

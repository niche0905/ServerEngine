#include "pch.h"
#include "CombatComponent.h"
#include "Content/Object/Actor/Pawn.h"

/*-------------------
   CombatComponent
-------------------*/

void CombatComponent::Init(ObjectId owner, Pawn* ownerPawn)
{
   SetOwner(owner);
   SetOwnerPawn(ownerPawn);
}

void CombatComponent::SetOwnerPawn(Pawn* pawn)
{
   ownerPawn_ = pawn;
   if (ownerPawn_) {
      if (ownerPawn_->GetId() != GetOwner()) {
         consoleLogger->Log(Color::Red, L"[CombatComponent] SetOwnerPawn: Owner ID mismatch (Owner: %u, Pawn's Owner: %u)\n", GetOwner().value, ownerPawn_->GetId().value);
      }
   }
}

Pawn* CombatComponent::GetOwnerPawn() const
{
   return ownerPawn_;
}

bool CombatComponent::CanAttack(const AttackRequest& request) const
{
   (void)request;   // 현재는 request를 사용하지 않지만, 향후 공격 유형이나 상황에 따른 조건을 추가할 때 사용할 수 있음
   
   if (ownerPawn_ == nullptr)
      return false;
   
   if (!ownerPawn_->IsAlive())
      return false;   // 죽은 Pawn은 공격할 수 없음
   
   return true;
}

bool CombatComponent::TryAttack(AttackRequest& request)
{
   if (not CanAttack(request)) {
      return false;   // 공격할 수 없는 상태
   }
   
   return ExecuteAttack(request);
}

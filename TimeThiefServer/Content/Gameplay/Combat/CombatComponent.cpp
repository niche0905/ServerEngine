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

bool CombatComponent::CanAttack() const
{
   if (ownerPawn_ == nullptr)
      return false;
   
   if (!ownerPawn_->IsAlive())
      return false;   // 죽은 Pawn은 공격할 수 없음
   
   return true;
}

bool CombatComponent::TryAttack(const AttackRequest& request)
{
   if (not CanAttack()) {
      return false;   // 공격할 수 없는 상태
   }
   
   return ExecuteAttack(request);
}

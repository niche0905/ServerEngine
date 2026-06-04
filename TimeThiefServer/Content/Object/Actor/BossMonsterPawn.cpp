#include "pch.h"
#include "BossMonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Content/Object/ObjectManager.h"

namespace
{
   constexpr float FullDamageRange = 1000.0f;
   constexpr float FullDamageRangeSq = FullDamageRange * FullDamageRange;

   ObjectId ResolveDamageDealerId(const DamageContext& ctx)
   {
      return ctx.instigator != ObjectId{}
         ? ctx.instigator
         : ctx.attacker;
   }
}

DamageResult BossMonsterPawn::ApplyDamage(ObjectManager& om, int32 amount, const DamageContext& ctx)
{
   const ObjectId dealerId = ResolveDamageDealerId(ctx);
   if (dealerId != ObjectId{}) {
      Pawn* dealer = om.FindAs<Pawn>(dealerId);
      if (dealer != nullptr) {
         SE::Math::Vector3 diff = dealer->GetPosition() - GetPosition();
         diff.z = 0.0f;

         if (diff.LengthSq() > FullDamageRangeSq) {
            DamageResult result;
            result.requested = amount;
            return result;
         }
      }
   }

   return MonsterPawn::ApplyDamage(om, amount, ctx);
}

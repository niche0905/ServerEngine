#pragma once
#include "MonsterPawn.h"

class BossMonsterPawn : public MonsterPawn
{
public:
   explicit BossMonsterPawn(int32 templateId)
      : MonsterPawn(templateId)
   {
   }

   virtual DamageResult ApplyDamage(ObjectManager& om, int32 amount, const DamageContext& ctx) override;
};

#pragma once
#include "DamageTypes.h"

class ObjectManager;

/*----------------
   IDamageable
----------------*/
//
// IDamageable은 피해를 입을 수 있는 객체가 구현해야 하는 인터페이스입니다.
// HealthComponent와 같은 컴포넌트를 이용하도록 설계되었습니다.
//

class IDamageable
{
public:
   virtual ~IDamageable() = default;
   
   virtual DamageResult ApplyDamage(ObjectManager& om, int32 amount, const DamageContext& ctx) = 0;
   
   virtual bool IsHpAlive() const = 0;
   virtual int32 GetHp() const = 0;
   virtual int32 GetMaxHp() const = 0;
   
};
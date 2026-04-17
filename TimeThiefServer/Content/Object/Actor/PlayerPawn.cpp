#include "pch.h"
#include "PlayerPawn.h"
#include "Content/Gameplay/Collider/ColliderComponent.h"
#include "Content/Gameplay/Combat/PlayerCombatComponent.h"
#include "Physics/Collider/CharacterCapsuleCollider.h"
#include "Service/Room/Room.h"

/*--------------
   PlayerPawn
--------------*/

int32 PlayerPawn::ResolveIncomingDamage(int32 amount, const DamageContext& ctx)
{
   constexpr int32 kMultiplierForZoneDamage = 10;   // TEMP: 존 데미지에 대한 데미지 배율 (예: 1 데미지당 10 화폐 삭제)
   
   if (ctx.type == DamageType::Zone) {
      auto room = GetRoom();
      if (!room)
         return amount;   // 방이 없는 경우에는 존 데미지 처리하지 않음
      
      const int64 deleteMoney = static_cast<int>(amount) * kMultiplierForZoneDamage;
      
      if (wallet_.CanSpend(CurrencyType::TimePoint, deleteMoney)) {
         wallet_.SpendMoney(room->GetObjectManager(), CurrencyType::TimePoint, deleteMoney, MoneyChangeContext{
            .reason = MoneyChangeReason::ZoneDamage,
         });
         return 0;   // 존 데미지로 삭제할 재화가 충분하면 체력에는 데미지를 주지 않음
      }
      
      // TODO: 존 데미지는 체력이 아니라 재화를 달게 한다 
      //       만약 재화의 차감으로도 죽을 수 있다면 사망 처리 진행
      //       재화가 모두 삭제되었더라도 죽이지 않을 것이라면 차감된 재화 양에 따라 amount 수정
      
      int64 currentMoney = wallet_.GetBalance(CurrencyType::TimePoint);
      wallet_.SpendMoney(room->GetObjectManager(), CurrencyType::TimePoint, currentMoney, MoneyChangeContext{
           .reason = MoneyChangeReason::ZoneDamage,
        });
      int64 remainDelete = deleteMoney - currentMoney;
      int32 remainAmount = static_cast<int32>((remainDelete + kMultiplierForZoneDamage - 1) / kMultiplierForZoneDamage);
      
      return remainAmount;   // 존 데미지로 삭제할 재화가 부족해서 체력에도 데미지를 줘야 하는 경우 남은 데미지 양 반환
   }
   
   return Pawn::ResolveIncomingDamage(amount, ctx);
}

void PlayerPawn::Damaged(const DamageResult& dmgResult)
{
   Pawn::Damaged(dmgResult);
   if (auto room = GetRoom()) {
      room->NotifyHealthChange(GetOwnerPlayerId(), dmgResult.hpAfter, -dmgResult.applied);
   }
}

bool PlayerPawn::IsConsumable(ItemId itemId) const
{
   // TODO: 아이템 데이터베이스 조회 후 소모품 여부 확인
   (void)itemId;
   return true;
}

InventoryOpResult PlayerPawn::AddItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
   return inventory_.AddItem(om, itemId, count, ctx);
}

InventoryOpResult PlayerPawn::RemoveItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
   return inventory_.RemoveItem(om, itemId, count, ctx);
}

InventoryOpResult PlayerPawn::ConsumeItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
   return inventory_.ConsumeItem(om, itemId, count, ctx);
}

MoneyChangeResult PlayerPawn::AddMoney(ObjectManager& om, CurrencyType currency, CurrencyAmount amount,
   const MoneyChangeContext& ctx)
{
   return wallet_.AddMoney(om, currency, amount, ctx);
}

MoneyChangeResult PlayerPawn::SpendMoney(ObjectManager& om, CurrencyType currency, CurrencyAmount amount,
   const MoneyChangeContext& ctx)
{
   return wallet_.SpendMoney(om, currency, amount, ctx);
}

void PlayerPawn::OnSpawn()
{
   Pawn::OnSpawn();
   
   combat_ = std::make_unique<PlayerCombatComponent>();
   if (combat_)
      combat_->Init(GetId(), this);
   
   // TEMP
   {
      auto bodyCollider = std::make_unique<ColliderComponent>();
      auto capsuleCollider = std::make_unique<SE::Physics::CharacterCapsuleCollider>(SE::Math::Vector3{0.0f, 0.0f, -90.0f}, 180.0f, 35.0f);
      bodyCollider->Init(GetId(), this, ColliderRole::Hurtbox, std::move(capsuleCollider));
      
      colliders_.push_back(std::move(bodyCollider));
   }
   
   dropOnDeath_.Init(GetId(), DropOnDeathPolicy{});
   respawn_.Init(GetId(), RespawnPolicy{});
   inventory_.Init(GetId(), 20);
   wallet_.Init(GetId());
}

void PlayerPawn::OnPreDestroy()
{
   Pawn::OnPreDestroy();
}

void PlayerPawn::Tick(float dt)
{
   Pawn::Tick(dt);
   
   (void)dt;
}

void PlayerPawn::OnDeath(ObjectManager& om, const DamageResult& dmgResult)
{
   Pawn::OnDeath(om, dmgResult);
   
   StartDeadState(om, dmgResult);
}

void PlayerPawn::StartDeadState(ObjectManager& om, const DamageResult& dmgResult)
{
   SetVelocity(Vector3{});
   
   DropOnDeathResult drops = dropOnDeath_.Generate(om, *this, DropOnDeathContext{
      .owner = GetId(),
      // .killer = dmgResult.instigator,
      // .nowMs = om.GetCurrentTimeMs(),
      .reason = DropReason::Death
   });
   
   // TODO: 드롭된 아이템과 화폐를 월드에 스폰하는 로직 추가
   // (om, drops);
}



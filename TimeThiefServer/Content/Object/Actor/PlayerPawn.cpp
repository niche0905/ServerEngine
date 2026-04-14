#include "pch.h"
#include "PlayerPawn.h"
#include "Content/Gameplay/Collider/ColliderComponent.h"
#include "Content/Gameplay/Combat/PlayerCombatComponent.h"
#include "Physics/Collider/CharacterCapsuleCollider.h"
#include "Service/Room/Room.h"

/*--------------
   PlayerPawn
--------------*/

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

MoneyChangeResult PlayerPawn::AddMoney(ObjectManager& om, CurrencyId currency, int64 amount,
   const MoneyChangeContext& ctx)
{
   return wallet_.AddMoney(om, currency, amount, ctx);
}

MoneyChangeResult PlayerPawn::SpendMoney(ObjectManager& om, CurrencyId currency, int64 amount,
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



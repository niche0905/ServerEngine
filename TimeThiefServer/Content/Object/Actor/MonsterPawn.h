#pragma once
#include "Pawn.h"
#include "Content/Gameplay/Drop/DropOnDeathComponent.h"
#include "Content/Gameplay/Spawn/RespawnComponent.h"

class ObjectManager;

// BT 라이브러리 wrapper 전방 선언 (아직 구현 안 됨)
class BTBrain;

/*---------------
   MonsterPawn
---------------*/
//
// MonsterPawn는 Pawn의 하위 클래스이며 몬스터에 특화된 기능을 제공합니다.
// 죽음 시 아이템 드롭과 리스폰 메커니즘을 포함합니다
// 추후에 작성될 BT 관련 기능도 포함할 예정입니다.
//

class MonsterPawn : public Pawn
                  , public IRespawnOwner
                  , public IDropOnDeathOwner
{
public:
   MonsterPawn() = default;
   virtual ~MonsterPawn() = default;
   
   MonsterPawn(const MonsterPawn&) = delete;
   MonsterPawn& operator=(const MonsterPawn&) = delete;
   
public:
   int32 GetTemplateId() const { return templateId_; }
   void SetTemplateId(int32 templateId) { templateId_ = templateId; }
   
   const Vector3& GetHomePosition() const { return respawn_.GetRespawnPosition(); }
   void SetHomePosition(const Vector3& pos) { respawn_.SetRespawnPosition(pos); }
   
   bool CanRespawn() const { return respawn_.IsEnabled(); }
   
   uint32 GetRespawnDelayMs() const { return respawn_.GetPolicy().delayMs; }
   // void SetRespawnDelayMs(uint32 delayMs) { respawn_.GetPolicy().delayMs = delayMs; }
   // Respawn 시간을 동적으로 변경하는 것은 제공되지 않음 (몬스터의 경우) RespawnPolicy를 설정할 때 한번만 설정하도록
   
public:
   virtual Vector3 ResolveRespawnPosition(ObjectManager& om) override;
   
   virtual void OnPreRespawn(ObjectManager& om) override;
   virtual void OnPostRespawn(ObjectManager& om) override;
   virtual void ApplyRespawnToWorld(ObjectManager& om, const Vector3& pos) override;
   virtual void GrantSpawnInvulnerability(ObjectManager& om, uint32 durationMs) override;
   
// TODO: Drop 시스템을 손봐야 할 듯 싶다 (Inventory 기반 vs Loot 기반 정리 필요)
public:
   virtual InventoryComponent& GetInventory() override;
   virtual WalletComponent& GetWallet() override;
   
   virtual bool IsConsumable(ItemId itemId) const = 0;
   
public:
   // Update 룰에 따라 처리 (룸 Tick, 혹은 Pawn Tick)
   void UpdateAI(ObjectManager& om, float dt);
   
protected:
   virtual void OnSpawn() override;
   virtual void Tick(float dt) override;
   virtual void OnPreDestroy() override;
   
protected:
   virtual void OnDeath(ObjectManager& om, const DamageResult& dmgResult) override;
   virtual bool ShouldRequestDestroyOnDeath() const override { return false; }
   // 죽은 상태 유지 후 리스폰 하도록 (Destroy 요청하지 않음)
   
private:
   void StartDeadState(ObjectManager& om, const DamageResult& dmgResult);
   void ScheduleRespawn(ObjectManager& om, uint64 nowMs);
   void ExecuteRespawn(ObjectManager& om, uint64 nowMs);
   
private:
   int32 templateId_{ 0 };
   
   BTBrain* brain_{ nullptr };
   
   RespawnComponent respawn_;
   DropOnDeathComponent drop_;
    
};

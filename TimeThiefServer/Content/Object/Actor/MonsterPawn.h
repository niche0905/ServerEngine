#pragma once
#include "Pawn.h"
#include "Content/Gameplay/AI/MonsterAiComponent.h"
#include "Content/Gameplay/Loot/ILootSource.h"
#include "Content/Gameplay/Loot/LootSourceComponent.h"

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
                  , public ILootSource
{
public:
   MonsterPawn() = default;
   virtual ~MonsterPawn() = default;
   
   MonsterPawn(const MonsterPawn&) = delete;
   MonsterPawn& operator=(const MonsterPawn&) = delete;
   
public:
   virtual se::common::ObjectType GetObjectType() const override { return se::common::OBJ_NPC; }
   
public:
   int32 GetTemplateId() const { return templateId_; }
   void SetTemplateId(int32 templateId) { templateId_ = templateId; }
   
public:
   virtual bool CanGenerateLoot() const override { return loot_.CanGenerateLoot(); }
   virtual LootSourceResult GenerateLoot(ObjectManager& om, LootTableService& service, const LootSourceContext& ctx) override;
   
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
   // void ScheduleRespawn(ObjectManager& om, uint64 nowMs);
   // void ExecuteRespawn(ObjectManager& om, uint64 nowMs);
   
private:
   int32 templateId_{ 0 };
   
   BTBrain* brain_{ nullptr };
   
   LootSourceComponent loot_;
   
   MonsterAiComponent ai_;
    
};

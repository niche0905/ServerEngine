#pragma once
#include "Pawn.h"
#include "Content/Gameplay/AI/MonsterAiComponent.h"
#include "Content/Gameplay/Loot/LootSourceComponent.h"

class ObjectManager;

/*---------------
   MonsterPawn
---------------*/
//
// MonsterPawn는 Pawn의 하위 클래스이며 몬스터에 특화된 기능을 제공합니다.
// 죽음 시 아이템 드롭과 리스폰 메커니즘을 포함합니다
// 추후에 작성될 BT 관련 기능도 포함할 예정입니다.
//

class MonsterPawn : public Pawn
{
public:
   explicit MonsterPawn(int32 templateId)
   : templateId_(templateId)
   {
   }
   
   MonsterPawn() = delete;
   virtual ~MonsterPawn() = default;
   
   MonsterPawn(const MonsterPawn&) = delete;
   MonsterPawn& operator=(const MonsterPawn&) = delete;
   
public:
   virtual se::common::ObjectType GetObjectType() const override { return se::common::OBJ_MONSTER; }
   
public:
   int32 GetTemplateId() const { return templateId_; }
   
   ObjectId GetTargetId() const { return ai_.GetTargetId(); }
   
   void SetTarget(Pawn* pawn);
   void ClearTarget();
   
public:
   virtual DamageResult ApplyDamage(ObjectManager& om, int32 amount, const DamageContext& ctx) override;
   
// IDropOwner
public:
   virtual LootBundle GenerateDrops() override;
   
public:
   // Update 룰에 따라 처리 (룸 Tick, 혹은 Pawn Tick)
   void UpdateAI(ObjectManager& om, float dt);
   
protected:
   virtual void OnSpawn() override;
   virtual void Tick(float dt) override;
   virtual void OnPreDestroy() override;
   
public:
   virtual void OnPreRespawn(ObjectManager& om) override;
   virtual void OnPostRespawn(ObjectManager& om) override;
   
public:
   void MoveTo(const Vector3& targetPos, float acceptRadius = 30.0f);
   void MoveAlongPath(std::vector<Vector3> path, float acceptRadius = 30.0f);
   void StopMove();
   
private:
   void UpdateMove(float dt);
   
public:
   void StartAI();
   void StopAI();
   
protected:
   virtual void OnDeath(ObjectManager& om, const DamageContext& ctx, const DamageResult& dmgResult) override;
   
   virtual bool ShouldRequestDestroyOnDeath() const override { return false; }
   // 죽은 상태 유지 후 리스폰 하도록 (Destroy 요청하지 않음)
   
private:
   void StartDeadState(ObjectManager& om, const DamageResult& dmgResult);
   // void ScheduleRespawn(ObjectManager& om, uint64 nowMs);
   // void ExecuteRespawn(ObjectManager& om, uint64 nowMs);
   
private:
   int32 templateId_{ 0 };
   
   LootSourceComponent loot_;
   MonsterAiComponent ai_;
   
   bool hasMovePath_{ false };
   std::vector<Vector3> movePath_;
   size_t movePathIndex_{ 0 };

   Vector3 finalMoveTarget_{};
   float moveSpeed_{ 600.0f };
   float moveAcceptRadius_{ 30.0f };
   float waypointAcceptRadius_{ 40.0f };
    
};

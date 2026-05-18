#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/behavior_tree.h"
#include "Content/Object/ObjectId.h"

class MonsterPawn;
class ObjectManager;

/*----------------------
   MonsterAiComponent
----------------------*/
//
// MonsterAiComponent는 MonsterPawn에 부착되어 몬스터의 행동을 제어하는 컴포넌트입니다.
//

class MonsterAiComponent
{
public:
   MonsterAiComponent() = default;
   ~MonsterAiComponent() = default;
   
   MonsterAiComponent(const MonsterAiComponent&) = delete;
   MonsterAiComponent& operator=(const MonsterAiComponent&) = delete;
   
public:
   bool Initialize(MonsterPawn* owner, ObjectManager* objectManager, uint32 npcId);
   void Shutdown();
   
   void Tick(float dt);
   
public:
   MonsterPawn* GetOwner() { return owner_;}
   const MonsterPawn* GetOwner() const { return owner_; }
   
   ObjectManager* GetObjectManager() { return objectManager_; }
   const ObjectManager* GetObjectManager() const { return objectManager_; }
   
   BT::Blackboard::Ptr GetBlackboard() { return blackboard_; }
   const BT::Blackboard::Ptr GetBlackboard() const { return blackboard_; }
   
public:
   void Start();
   void Stop();
   bool IsRunning() const { return running_; }
   
public:
   ObjectId GetTargetId() const { return targetId_; }
   void SetTargetId(ObjectId targetId) { targetId_ = targetId; }
   
   void ClearTarget() { targetId_ = ObjectId{}; }
   bool HasTarget() const { return targetId_ != ObjectId{}; }
   
// BT node API
public:
   // bool TryFindTarget();
   // bool IsTargetAlive() const;
   // bool IsTargetInRange(float range) const;
   //
   // bool RequestMoveToTarget(float acceptanceRadius);
   // bool RequestMoveToHome(float acceptanceRadius);
   //
   // bool CanAttack() const;
   // bool TryAttack();
   
private:
   void PushRuntimeStateToBlackboard(float dt);
   
private:
   MonsterPawn* owner_{ nullptr };
   ObjectManager* objectManager_{ nullptr };
   
   bool running_{ false };
   
   ObjectId targetId_{};
   
   BT::Blackboard::Ptr blackboard_;
   std::unique_ptr<BT::Tree> tree_;
    
};

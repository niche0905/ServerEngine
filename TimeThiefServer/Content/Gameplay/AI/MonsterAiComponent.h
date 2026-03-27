#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "behaviortree_cpp/bt_factory.h"
#include "behaviortree_cpp/blackboard.h"
#include "behaviortree_cpp/behavior_tree.h"

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
   bool Initialize(MonsterPawn* owner, ObjectManager* objectManager, std::string_view treeXmlPath);
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
   bool IsInitialized() const { return initialized_; }
   
   void Start();
   void Stop();
   bool IsRunning() const { return running_; }
   
public:
   uint64 GetTargetId() const { return targetId_; }
   void SetTargetId(uint64 targetId) { targetId_ = targetId; }
   
   void ClearTarget() { targetId_ = 0; }
   bool HasTarget() const { return targetId_ != 0; }
   
// BT node API
public:
   bool TryFindTarget();
   bool IsTargetAlive() const;
   bool IsTargetInRange(float range) const;
   
   bool RequestMoveToTarget(float acceptanceRadius);
   bool RequestMoveToHome(float acceptanceRadius);
   
   bool CanAttack() const;
   bool TryAttack();
   
private:
   bool RegisterNodes();
   bool CreateTreeFromFile(std::string_view treeXmlPath);
   void PushRuntimeStateToBlackboard(float dt);
   
private:
   MonsterPawn* owner_{ nullptr };
   ObjectManager* objectManager_{ nullptr };
   
   bool initialized_{ false };
   bool running_{ false };
   
   uint64 targetId_{ 0 };
   
   BT::BehaviorTreeFactory factory_;
   BT::Blackboard::Ptr blackboard_;
   BT::Tree tree_;
    
};

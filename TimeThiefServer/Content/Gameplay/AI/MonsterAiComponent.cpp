#include "pch.h"
#include "MonsterAiComponent.h"

/*----------------------
   MonsterAiComponent
----------------------*/

bool MonsterAiComponent::Initialize(MonsterPawn* owner, ObjectManager* objectManager, std::string_view treeXmlPath)
{
   if (initialized_)
      return false;
   
   if (owner == nullptr or objectManager == nullptr or treeXmlPath.empty())
      return false;
   
   owner_ = owner;
   objectManager_ = objectManager;
   blackboard_ = BT::Blackboard::create();
   
   blackboard_->set("self", owner_);
   blackboard_->set("object_manager", objectManager_);
   blackboard_->set("target_id", targetId_);
   
   if (!RegisterNodes())
      return false;
   
   if (!CreateTreeFromFile(treeXmlPath))
      return false;
   
   initialized_ = true;
   running_ = true;
   return true;
}

void MonsterAiComponent::Shutdown()
{
   running_ = false;
   initialized_ = false;
   targetId_ = 0;
   
   tree_ = BT::Tree{};
   blackboard_.reset();
   
   objectManager_ = nullptr;
   owner_ = nullptr;
}

void MonsterAiComponent::Tick(float dt)
{
   if (!initialized_ or !running_)
      return;
   
   PushRuntimeStateToBlackboard(dt);
   
   tree_.tickOnce();
}

void MonsterAiComponent::Start()
{
   if (initialized_)
      running_ = true;
}

void MonsterAiComponent::Stop()
{
   running_ = false;
}

bool MonsterAiComponent::TryFindTarget()
{
   // TODO: owner_ 주변에 Target 검색 (있다면 targetId 갱신)
   return false;
}

bool MonsterAiComponent::IsTargetAlive() const
{
   if (targetId_ == 0 or objectManager_ == nullptr)
      return false;
   
   // TODO: targetId_에 해당하는 Object가 존재하는지, 그리고 살아있는지 확인
   return true;
}

bool MonsterAiComponent::IsTargetInRange(float range) const
{
    if (targetId_ == 0 or owner_ == nullptr or objectManager_ == nullptr)
      return false;
   
   // TODO: owner_와 targetId_에 해당하는 Object 간의 거리를 계산하여 range 이내인지 확인
   return false;
}

bool MonsterAiComponent::RequestMoveToTarget(float acceptanceRadius)
{
   if (targetId_ == 0 or owner_ == nullptr)
      return false;
   
   // TODO: target 위치를 얻어서 해당 방향으로 owner_를 이동시키는 명령을 ObjectManager에 요청 (acceptanceRadius 이내에 도달하면 성공)
   return false;
}

bool MonsterAiComponent::RequestMoveToHome(float acceptanceRadius)
{
   if (owner_ == nullptr)
      return false;
   
   // TODO: owner_의 home 위치를 얻어서 해당 방향으로 owner_를 이동시키는 명령을 ObjectManager에 요청 (acceptanceRadius 이내에 도달하면 성공)
   return false;
}

bool MonsterAiComponent::CanAttack() const
{
   if (owner_ == nullptr)
      return false;
   
   // TODO: 공격 가능 여부 판단 (예: 공격 쿨다운, 체력 상태 등)
   return true;
}

bool MonsterAiComponent::TryAttack()
{
   if (!CanAttack())
      return false;
   
   // TODO: targetId_에 해당하는 Object에 공격 명령을 ObjectManager에 요청 (성공 여부 반환)
   return true;
}

bool MonsterAiComponent::RegisterNodes()
{
   // ex)
   // factory_.registerNodeType<BT_FindTarget>("FindTarget");
   
   return false;
}

bool MonsterAiComponent::CreateTreeFromFile(std::string_view treeXmlPath)
{
   try
   {
      tree_ = factory_.createTreeFromFile(std::string(treeXmlPath), blackboard_);
      return true;
   }
   catch (...)
   {
      return false;
   }
}

void MonsterAiComponent::PushRuntimeStateToBlackboard(float dt)
{
   blackboard_->set("dt", dt);
   blackboard_->set("target_id", targetId_);
}

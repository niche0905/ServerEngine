#include "pch.h"
#include "MonsterAiComponent.h"
#include "Content/Object/ObjectId.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Data/GameDataManager.h"
#include "Data/AI/AiBlackboard.h"
#include "Service/Room/Room.h"
#include "Data/AI/AiManager.h"
#include "Shard/GameShard.h"

namespace BB = AiBlackboardKey;
class Pawn;

/*----------------------
   MonsterAiComponent
----------------------*/

bool MonsterAiComponent::Initialize(MonsterPawn* owner, ObjectManager* objectManager, uint32 npcId)
{
   if (owner == nullptr or objectManager == nullptr)
      return false;
   
   owner_ = owner;
   objectManager_ = objectManager;
   blackboard_ = BT::Blackboard::create();
   
   blackboard_->set<MonsterPawn*>(BB::SelfNpc, owner_);
   
   ResetBlackboard();
   
   auto room = owner_->GetRoom();
   if (room == nullptr)
      return false;
   
   auto ownerShard = room->GetOwnerShard();
   if (ownerShard == nullptr)
      return false;
   
   tree_ = std::make_unique<BT::Tree>(ownerShard->CreateAiTree(npcId, blackboard_));
   
   running_ = true;
   return true;
}

void MonsterAiComponent::Shutdown()
{
   running_ = false;
   targetId_ = ObjectId{};
   
   tree_ = nullptr;
   blackboard_.reset();
   
   objectManager_ = nullptr;
   owner_ = nullptr;
}

void MonsterAiComponent::Tick(float dt)
{
   if (!running_)
      return;
   
   if (owner_ == nullptr)
      return;
   
   if (tree_ == nullptr)
      return;
   
   PushRuntimeStateToBlackboard(dt);
   
   tree_->tickOnce();
}

void MonsterAiComponent::Start()
{
   running_ = true;
}

void MonsterAiComponent::Stop()
{
   running_ = false;
}

void MonsterAiComponent::HaltTree()
{
   if (tree_)
   {
      tree_->haltTree();
   }
}

void MonsterAiComponent::ResetBlackboard()
{
   if (blackboard_ == nullptr or owner_ == nullptr)
      return;
   
   blackboard_->set<CombatEventType>(BB::CombatMode, CombatEventType::None);
   blackboard_->set<ObjectId>(BB::TargetId, ObjectId{});
   blackboard_->set<Pawn*>(BB::TargetPawn, nullptr);
   
   owner_->SetTarget(nullptr);
}

// bool MonsterAiComponent::TryFindTarget()
// {
//    // TODO: owner_ 주변에 Target 검색 (있다면 targetId 갱신)
//    return false;
// }
//
// bool MonsterAiComponent::IsTargetAlive() const
// {
//    if (targetId_ == 0 or objectManager_ == nullptr)
//       return false;
//    
//    // TODO: targetId_에 해당하는 Object가 존재하는지, 그리고 살아있는지 확인
//    return true;
// }
//
// bool MonsterAiComponent::IsTargetInRange(float range) const
// {
//     if (targetId_ == 0 or owner_ == nullptr or objectManager_ == nullptr)
//       return false;
//    
//    // TODO: owner_와 targetId_에 해당하는 Object 간의 거리를 계산하여 range 이내인지 확인
//    return false;
// }
//
// bool MonsterAiComponent::RequestMoveToTarget(float acceptanceRadius)
// {
//    if (targetId_ == 0 or owner_ == nullptr)
//       return false;
//    
//    // TODO: target 위치를 얻어서 해당 방향으로 owner_를 이동시키는 명령을 ObjectManager에 요청 (acceptanceRadius 이내에 도달하면 성공)
//    return false;
// }
//
// bool MonsterAiComponent::RequestMoveToHome(float acceptanceRadius)
// {
//    if (owner_ == nullptr)
//       return false;
//    
//    // TODO: owner_의 home 위치를 얻어서 해당 방향으로 owner_를 이동시키는 명령을 ObjectManager에 요청 (acceptanceRadius 이내에 도달하면 성공)
//    return false;
// }
//
// bool MonsterAiComponent::CanAttack() const
// {
//    if (owner_ == nullptr)
//       return false;
//    
//    // TODO: 공격 가능 여부 판단 (예: 공격 쿨다운, 체력 상태 등)
//    return true;
// }
//
// bool MonsterAiComponent::TryAttack()
// {
//    if (!CanAttack())
//       return false;
//    
//    // TODO: targetId_에 해당하는 Object에 공격 명령을 ObjectManager에 요청 (성공 여부 반환)
//    return true;
// }

void MonsterAiComponent::PushRuntimeStateToBlackboard(float dt)
{
}

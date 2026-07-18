#include "pch.h"
#include "CatAcquireOrValidateTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace 
{
    constexpr float AcquireRange = 3000.0f;
    
    constexpr float LoseRange = 10000.0f;
    constexpr float LoseRangeSq = LoseRange * LoseRange;

    void ResetCombatMode(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
    }

    void ClearTargetState(BT::TreeNode& node, MonsterPawn* selfNpc)
    {
        ResetCombatMode(node);

        node.setOutput<ObjectId>(BB::TargetId, ObjectId{});
        node.setOutput<Pawn*>(BB::TargetPawn, nullptr);

        if (selfNpc) {
            selfNpc->ClearTarget();
        }
    }
}


BT::PortsList CatAcquireOrValidateTargetNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::BidirectionalPort<ObjectId>(BB::TargetId),
        BT::BidirectionalPort<Pawn*>(BB::TargetPawn),
        BT::BidirectionalPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatAcquireOrValidateTargetNode::tick()
{
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) || selfNpc == nullptr) {
        ClearTargetState(*this, nullptr);
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc->IsDead()) {
        ClearTargetState(*this, selfNpc);
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc->GetRoom();
    if (room == nullptr) {
        ClearTargetState(*this, selfNpc);
        return BT::NodeStatus::FAILURE;
    }
    
    CombatEventType mode = CombatEventType::None;
    getInput<CombatEventType>(BB::CombatMode, mode);

    const bool keepDeadTargetDuringAttack =
        mode == CombatEventType::CatClaw ||
        mode == CombatEventType::CatBite;

    // 1. 캐싱된 Pawn 우선 검증
    Pawn* targetPawn = nullptr;
    if (getInput<Pawn*>(BB::TargetPawn, targetPawn) && targetPawn != nullptr)
    {
        if (targetPawn->IsDead())
        {
            if (keepDeadTargetDuringAttack) {
                return BT::NodeStatus::SUCCESS;
            }

            ClearTargetState(*this, selfNpc);
            return BT::NodeStatus::FAILURE;
        }

        const float distSq =
            (targetPawn->GetPosition() - selfNpc->GetPosition()).LengthSq();

        if (distSq <= LoseRangeSq || keepDeadTargetDuringAttack)
        {
            setOutput<ObjectId>(BB::TargetId, targetPawn->GetId());
            selfNpc->SetTarget(targetPawn);
            return BT::NodeStatus::SUCCESS;
        }

        ClearTargetState(*this, selfNpc);
        return BT::NodeStatus::FAILURE;
    }

    // 2. TargetId 기반 재검증
    ObjectId targetId{};
    if (getInput<ObjectId>(BB::TargetId, targetId) && targetId != ObjectId{})
    {
        Pawn* foundTarget = room->GetObjectManager().FindAs<Pawn>(targetId);
        if (foundTarget != nullptr && !foundTarget->IsDead())
        {
            const float distSq =
                (foundTarget->GetPosition() - selfNpc->GetPosition()).LengthSq();

            if (distSq <= LoseRangeSq)
            {
                setOutput<Pawn*>(BB::TargetPawn, foundTarget);
                selfNpc->SetTarget(foundTarget);
                return BT::NodeStatus::SUCCESS;
            }
        }

        // TargetId가 있었는데 더 이상 유효하지 않은 경우는 진짜 타겟 상실
        ClearTargetState(*this, selfNpc);
    }

    // 3. 새 타겟 탐색
    Pawn* bestTarget = selfNpc->SelectTarget(AcquireRange);

    if (bestTarget == nullptr)
    {
        // 여기서는 CombatMode만 초기화해도 됨.
        // 이미 위에서 유효하지 않은 TargetId/TargetPawn은 ClearTargetState로 처리했기 때문.
        ResetCombatMode(*this);
        return BT::NodeStatus::FAILURE;
    }

    setOutput<ObjectId>(BB::TargetId, bestTarget->GetId());
    setOutput<Pawn*>(BB::TargetPawn, bestTarget);
    selfNpc->SetTarget(bestTarget);

    return BT::NodeStatus::SUCCESS;
}

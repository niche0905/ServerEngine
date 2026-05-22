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
    constexpr float AcquireRangeSq = AcquireRange * AcquireRange;
    
    constexpr float LoseRange = 10000.0f;
    constexpr float LoseRangeSq = LoseRange * LoseRange;

    void ResetCombatReservation(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
        node.setOutput<ObjectId>(BB::TargetId, ObjectId{});
        node.setOutput<Pawn*>(BB::TargetPawn, nullptr);
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
        ResetCombatReservation(*this);
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc->IsDead()) {
        ResetCombatReservation(*this);
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc->GetRoom();
    if (room == nullptr) {
        ResetCombatReservation(*this);
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

            ResetCombatReservation(*this);
            return BT::NodeStatus::FAILURE;
        }

        const float distSq =
            (targetPawn->GetPosition() - selfNpc->GetPosition()).LengthSq();

        if (distSq <= LoseRangeSq || keepDeadTargetDuringAttack) {
            setOutput<ObjectId>(BB::TargetId, targetPawn->GetId());
            return BT::NodeStatus::SUCCESS;
        }

        ResetCombatReservation(*this);
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
                return BT::NodeStatus::SUCCESS;
            }
        }

        ResetCombatReservation(*this);
    }

    // 3. 새 타겟 탐색
    Pawn* bestTarget = nullptr;
    float bestDistSq = AcquireRangeSq;

    const SE::Math::Vector3 selfPos = selfNpc->GetPosition();

    room->GetObjectManager().ForEachAlive([&](BaseObject* obj)
    {
        auto* pawn = dynamic_cast<Pawn*>(obj);
        if (pawn == nullptr) {
            return;
        }

        if (pawn->GetId() == selfNpc->GetId()) {
            return;
        }

        if (pawn->IsDead()) {
            return;
        }

        if (pawn->GetObjectType() != ObjectType::OBJ_PLAYER) {
            return;
        }

        const float distSq = (pawn->GetPosition() - selfPos).LengthSq();
        if (distSq > AcquireRangeSq) {
            return;
        }

        if (distSq < bestDistSq)
        {
            bestDistSq = distSq;
            bestTarget = pawn;
        }
    });

    if (bestTarget == nullptr)
    {
        ResetCombatReservation(*this);
        return BT::NodeStatus::FAILURE;
    }

    setOutput<ObjectId>(BB::TargetId, bestTarget->GetId());
    setOutput<Pawn*>(BB::TargetPawn, bestTarget);

    return BT::NodeStatus::SUCCESS;
}

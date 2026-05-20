#include "pch.h"
#include "CatIsTargetInMeleeRangeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;


BT::PortsList CatIsTargetInMeleeRangeNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn)
    };
}

BT::NodeStatus CatIsTargetInMeleeRangeNode::tick()
{
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) || selfNpc == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    Pawn* targetPawn = nullptr;
    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn) || targetPawn == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc->IsDead() || targetPawn->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    constexpr float MeleeRange = 180.0f;
    constexpr float MeleeRangeSq = MeleeRange * MeleeRange;

    const SE::Math::Vector3 selfPos = selfNpc->GetPosition();
    const SE::Math::Vector3 targetPos = targetPawn->GetPosition() - SE::Math::Vector3(0.0f, 0.0f, 90.0f);

    const SE::Math::Vector3 diff = targetPos - selfPos;

    return diff.LengthSq() <= MeleeRangeSq
        ? BT::NodeStatus::SUCCESS
        : BT::NodeStatus::FAILURE;
}

#include "pch.h"
#include "CatMoveTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float MeleeEnterDistance = 180.0f;
    constexpr float MeleeEnterDistanceSq = MeleeEnterDistance * MeleeEnterDistance;

    constexpr float MoveArriveDistance = 80.0f;

    constexpr float RepathInterval = 0.25f;
    constexpr float TargetMoveRepathDistance = 120.0f;
    constexpr float TargetMoveRepathDistanceSq = TargetMoveRepathDistance * TargetMoveRepathDistance;

    constexpr float ApproachSideAngle = 35.0f * Pi / 180.0f;
    constexpr float ApproachDistance = 180.0f;
    
    SE::Math::Vector3 Normalize2D(const SE::Math::Vector3& v)
    {
        SE::Math::Vector3 result{ v.x, v.y, 0.0f };
        if (result.LengthSq() <= 0.0001f) {
            return SE::Math::Vector3{ 1.0f, 0.0f, 0.0f };
        }
        return result.Normalized();
    }

    SE::Math::Vector3 Rotate2D(const SE::Math::Vector3& v, float rad)
    {
        const float c = std::cos(rad);
        const float s = std::sin(rad);

        return {
            v.x * c - v.y * s,
            v.x * s + v.y * c,
            0.0f
        };
    }
}

BT::PortsList CatMoveTargetNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::OutputPort<ObjectId>(BB::TargetId),
        BT::BidirectionalPort<Pawn*>(BB::TargetPawn)
    };
}

BT::NodeStatus CatMoveTargetNode::onStart()
{
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;

    moveGoal_ = {};
    lastTargetPos_ = {};
    elapsedRepath_ = RepathInterval;

    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc_) || selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn_) || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead() || targetPawn_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    orbitSide_ = (selfNpc_->GetId().value % 2 == 0) ? 1 : -1;
    lastTargetPos_ = targetPawn_->GetPosition();

    return onRunning();
}

BT::NodeStatus CatMoveTargetNode::onRunning()
{
    if (selfNpc_ == nullptr || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead() || targetPawn_->IsDead()) {
        selfNpc_->StopMove();
        setOutput<ObjectId>(BB::TargetId, ObjectId{});
        setOutput<Pawn*>(BB::TargetPawn, nullptr);
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    const SE::Math::Vector3 selfPos = selfNpc_->GetPosition();
    const SE::Math::Vector3 targetPos = targetPawn_->GetPosition() - SE::Math::Vector3{ 0.0f, 0.0f, 90.0f };

    SE::Math::Vector3 toTarget = targetPos - selfPos;
    toTarget.z = 0.0f;

    const float distSq = toTarget.LengthSq();

    if (distSq <= MeleeEnterDistanceSq)
    {
        selfNpc_->StopMove();
        return BT::NodeStatus::SUCCESS;
    }

    elapsedRepath_ += room->GetDelta();

    const bool targetMovedEnough =
        (targetPos - lastTargetPos_).LengthSq() >= TargetMoveRepathDistanceSq;

    const bool needRepath =
        elapsedRepath_ >= RepathInterval || targetMovedEnough;

    if (needRepath)
    {
        elapsedRepath_ = 0.0f;
        lastTargetPos_ = targetPos;

        const SE::Math::Vector3 targetToSelf = Normalize2D(selfPos - targetPos);

        // 정면으로 꽂히지 않도록 살짝 좌/우로 비틀어서 접근
        SE::Math::Vector3 approachDir =
            Rotate2D(targetToSelf, orbitSide_ * ApproachSideAngle);

        moveGoal_ = targetPos + approachDir * ApproachDistance;
        moveGoal_.z = targetPos.z;

        // 여기서는 목적지만 넘긴다.
        // SetMoveTarget 내부에서 FindPath 후 MoveAlongPath 호출.
        selfNpc_->MoveTo(moveGoal_, MoveArriveDistance);
    }

    return BT::NodeStatus::RUNNING;
}

void CatMoveTargetNode::onHalted()
{
    if (selfNpc_ != nullptr) {
        selfNpc_->StopMove();
    }

    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
    moveGoal_ = {};
    lastTargetPos_ = {};
    elapsedRepath_ = 0.0f;
    orbitSide_ = 1;
}

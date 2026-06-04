#include "pch.h"
#include "MinionMoveTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Data/GameDataManager.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float BoundaryRadius = 4000.0f;
    constexpr float BoundaryRadiusSq = BoundaryRadius * BoundaryRadius;

    constexpr float MeleeEnterDistance = 180.0f;
    constexpr float MeleeEnterDistanceSq = MeleeEnterDistance * MeleeEnterDistance;

    constexpr float MoveArriveDistance = 80.0f;
    constexpr float RepathInterval = 0.25f;
    constexpr float TargetMoveRepathDistance = 100.0f;
    constexpr float TargetMoveRepathDistanceSq = TargetMoveRepathDistance * TargetMoveRepathDistance;

    bool IsInsideBoundary(const SE::Math::Vector3& spawnPos, const SE::Math::Vector3& pos)
    {
        SE::Math::Vector3 diff = pos - spawnPos;
        diff.z = 0.0f;
        return diff.LengthSq() <= BoundaryRadiusSq;
    }
}

BT::PortsList MinionMoveTargetNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::OutputPort<ObjectId>(BB::TargetId),
        BT::BidirectionalPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus MinionMoveTargetNode::onStart()
{
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
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

    lastTargetPos_ = targetPawn_->GetPosition();
    return onRunning();
}

BT::NodeStatus MinionMoveTargetNode::onRunning()
{
    if (selfNpc_ == nullptr || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead() || targetPawn_->IsDead()) {
        selfNpc_->StopMove();
        setOutput<ObjectId>(BB::TargetId, ObjectId{});
        setOutput<Pawn*>(BB::TargetPawn, nullptr);
        selfNpc_->ClearTarget();
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    const SE::Math::Vector3 spawnPos = selfNpc_->GetSavedRespawnPosition();
    const SE::Math::Vector3 targetPos = targetPawn_->GetPosition() - SE::Math::Vector3{0.0f, 0.0f, 90.0f};

    if (!IsInsideBoundary(spawnPos, targetPos))
    {
        selfNpc_->StopMove();
        setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
        setOutput<ObjectId>(BB::TargetId, ObjectId{});
        setOutput<Pawn*>(BB::TargetPawn, nullptr);
        selfNpc_->ClearTarget();
        return BT::NodeStatus::FAILURE;
    }

    SE::Math::Vector3 toTarget = targetPos - selfNpc_->GetPosition();
    toTarget.z = 0.0f;

    if (toTarget.LengthSq() <= MeleeEnterDistanceSq) {
        selfNpc_->StopMove();
        return BT::NodeStatus::SUCCESS;
    }

    elapsedRepath_ += room->GetDelta();

    const bool targetMovedEnough =
        (targetPos - lastTargetPos_).LengthSq() >= TargetMoveRepathDistanceSq;

    if (elapsedRepath_ >= RepathInterval || targetMovedEnough)
    {
        elapsedRepath_ = 0.0f;
        lastTargetPos_ = targetPos;

        if (!TryMoveToGoal(targetPos))
        {
            selfNpc_->StopMove();
            setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
            return BT::NodeStatus::FAILURE;
        }
    }

    return BT::NodeStatus::RUNNING;
}

void MinionMoveTargetNode::onHalted()
{
    if (selfNpc_ != nullptr) {
        selfNpc_->StopMove();
    }

    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
    lastTargetPos_ = {};
    elapsedRepath_ = 0.0f;
}

bool MinionMoveTargetNode::TryMoveToGoal(const SE::Math::Vector3& goal)
{
    if (selfNpc_ == nullptr) {
        return false;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr || room->GetGameDataManager() == nullptr) {
        return false;
    }

    auto* navQueryContext = room->GetNavigationQueryContext();
    if (navQueryContext == nullptr) {
        return false;
    }

    const ServerMap& map = room->GetGameDataManager()->GetServerMap();

    SE::Math::Vector3 navGoal{};
    if (!map.ProjectToNavMesh(*navQueryContext, goal, navGoal)) {
        return false;
    }

    std::vector<SE::Math::Vector3> path;
    if (map.FindPath(*navQueryContext, selfNpc_->GetPosition(), navGoal, path) != NavPathResult::Success || path.empty()) {
        return false;
    }

    selfNpc_->MoveAlongPath(std::move(path), MoveArriveDistance);
    return true;
}

#include "pch.h"
#include "MinionReturnToSpawnNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Data/GameDataManager.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float ArriveDistance = 100.0f;
    constexpr float ArriveDistanceSq = ArriveDistance * ArriveDistance;
}

BT::PortsList MinionReturnToSpawnNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc)
    };
}

BT::NodeStatus MinionReturnToSpawnNode::onStart()
{
    selfNpc_ = nullptr;
    hasPath_ = false;

    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc_) || selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    return onRunning();
}

BT::NodeStatus MinionReturnToSpawnNode::onRunning()
{
    if (selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        selfNpc_->StopMove();
        return BT::NodeStatus::FAILURE;
    }

    const SE::Math::Vector3 spawnPos = selfNpc_->GetSavedRespawnPosition();
    SE::Math::Vector3 diff = spawnPos - selfNpc_->GetPosition();
    diff.z = 0.0f;

    if (diff.LengthSq() <= ArriveDistanceSq) {
        selfNpc_->StopMove();
        return BT::NodeStatus::SUCCESS;
    }

    if (!hasPath_ && !TryMoveToSpawn()) {
        return BT::NodeStatus::FAILURE;
    }

    return BT::NodeStatus::RUNNING;
}

void MinionReturnToSpawnNode::onHalted()
{
    if (selfNpc_ != nullptr) {
        selfNpc_->StopMove();
    }

    selfNpc_ = nullptr;
    hasPath_ = false;
}

bool MinionReturnToSpawnNode::TryMoveToSpawn()
{
    if (selfNpc_ == nullptr) {
        return false;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr || room->GetGameDataManager() == nullptr) {
        return false;
    }

    const ServerMap& map = room->GetGameDataManager()->GetServerMap();

    SE::Math::Vector3 navGoal{};
    if (!map.ProjectToNavMesh(selfNpc_->GetSavedRespawnPosition(), navGoal)) {
        return false;
    }

    std::vector<SE::Math::Vector3> path;
    if (map.FindPath(selfNpc_->GetPosition(), navGoal, path) != NavPathResult::Success || path.empty()) {
        return false;
    }

    selfNpc_->MoveAlongPath(std::move(path), ArriveDistance);
    hasPath_ = true;
    return true;
}

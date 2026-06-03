#include "pch.h"
#include "MinionIdleNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float MinIdleTime = 3.0f;
    constexpr float MaxIdleTime = 6.0f;

    float RandomFloat(float min, float max)
    {
        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }
}

BT::PortsList MinionIdleNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc)
    };
}

BT::NodeStatus MinionIdleNode::onStart()
{
    selfNpc_ = nullptr;
    elapsedTime_ = 0.0f;

    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc_) || selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    selfNpc_->StopMove();
    idleDuration_ = RandomFloat(MinIdleTime, MaxIdleTime);

    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus MinionIdleNode::onRunning()
{
    if (selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    elapsedTime_ += room->GetDelta();

    if (elapsedTime_ >= idleDuration_) {
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::RUNNING;
}

void MinionIdleNode::onHalted()
{
    elapsedTime_ = 0.0f;
    idleDuration_ = 0.0f;
    selfNpc_ = nullptr;
}

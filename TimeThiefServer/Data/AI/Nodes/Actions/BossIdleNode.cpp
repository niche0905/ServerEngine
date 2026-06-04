#include "pch.h"
#include "BossIdleNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;

namespace
{
    constexpr float MinIdleTime = 0.8f;
    constexpr float MaxIdleTime = 1.8f;

    float RandomFloat(float min, float max)
    {
        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }
}

BT::PortsList BossIdleNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc)
    };
}

BT::NodeStatus BossIdleNode::onStart()
{
    selfNpc_ = nullptr;
    elapsedTime_ = 0.0f;

    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc_) || selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    selfNpc_->StopMove();
    idleDuration_ = RandomFloat(MinIdleTime, MaxIdleTime);

    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus BossIdleNode::onRunning()
{
    if (selfNpc_ == nullptr || selfNpc_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    elapsedTime_ += room->GetDelta();
    return elapsedTime_ >= idleDuration_
        ? BT::NodeStatus::SUCCESS
        : BT::NodeStatus::RUNNING;
}

void BossIdleNode::onHalted()
{
    selfNpc_ = nullptr;
    elapsedTime_ = 0.0f;
    idleDuration_ = 0.0f;
}

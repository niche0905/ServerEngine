#include "pch.h"
#include "CatPatrolAroundSpawnNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Data/GameDataManager.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float PatrolRadius = 800.0f;
    constexpr float ArriveDistance = 100.0f;
    constexpr float ArriveDistanceSq = ArriveDistance * ArriveDistance;

    // 도착했을 때 Idle로 넘어갈 확률
    constexpr float IdleChanceOnArrive = 0.45f;

    constexpr int32 MaxFindPatrolPointAttempts = 8;

    SE::Math::Vector3 RandomPointInCircle(const SE::Math::Vector3& center, float radius)
    {
        static thread_local std::mt19937 rng(std::random_device{}());

        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
        std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * Pi);

        const float r = radius * std::sqrt(dist01(rng));
        const float theta = distAngle(rng);

        return {
            center.x + std::cos(theta) * r,
            center.y + std::sin(theta) * r,
            center.z
        };
    }
}


BT::PortsList CatPatrolAroundSpawnNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc)
    };
}

BT::NodeStatus CatPatrolAroundSpawnNode::onStart()
{
    selfNpc_ = nullptr;
    patrolGoal_ = {};
    hasGoal_ = false;
    elapsedRepath_ = 0.0f;

    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc_) || selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    return onRunning();
}

BT::NodeStatus CatPatrolAroundSpawnNode::onRunning()
{
    if (selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        selfNpc_->StopMove();
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    const SE::Math::Vector3 selfPos = selfNpc_->GetPosition();

    if (hasGoal_)
    {
        if ((patrolGoal_ - selfPos).LengthSq() <= ArriveDistanceSq)
        {
            selfNpc_->StopMove();

            if (AiBlackboard::RandomChance(IdleChanceOnArrive)) {
                hasGoal_ = false;
                return BT::NodeStatus::SUCCESS; // Fallback 다음 Idle 실행
            }

            hasGoal_ = false; // 새 순찰 지점 다시 뽑기
        }
        else {
            return BT::NodeStatus::RUNNING;
        }
    }

    const ServerMap& map = room->GetGameDataManager()->GetServerMap();
    const SE::Math::Vector3 spawnPos = selfNpc_->GetSavedRespawnPosition();

    for (int32 i = 0; i < MaxFindPatrolPointAttempts; ++i)
    {
        const SE::Math::Vector3 randomPos =
            RandomPointInCircle(spawnPos, PatrolRadius);

        SE::Math::Vector3 navPos{};
        if (!map.ProjectToNavMesh(randomPos, navPos)) {
            continue;
        }

        std::vector<SE::Math::Vector3> path;
        if (map.FindPath(selfPos, navPos, path) != NavPathResult::Success) {
            continue;
        }

        patrolGoal_ = navPos;
        hasGoal_ = true;

        selfNpc_->MoveAlongPath(std::move(path), ArriveDistance);
        return BT::NodeStatus::RUNNING;
    }

    return BT::NodeStatus::FAILURE;
}

void CatPatrolAroundSpawnNode::onHalted()
{
    if (selfNpc_ != nullptr) {
        selfNpc_->StopMove();
    }

    selfNpc_ = nullptr;
    patrolGoal_ = {};
    hasGoal_ = false;
    elapsedRepath_ = 0.0f;
}

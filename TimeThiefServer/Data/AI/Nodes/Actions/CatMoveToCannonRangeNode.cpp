#include "pch.h"
#include "CatMoveToCannonRangeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Data/GameDataManager.h"
#include "Physics/Ray/Ray.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float CannonDesiredDistance = 1500.0f;
    constexpr float CannonMinDistance = 900.0f;
    constexpr float CannonMaxDistance = 1800.0f;

    constexpr float ArriveDistance = 120.0f;
    constexpr float ArriveDistanceSq = ArriveDistance * ArriveDistance;

    constexpr float RepathInterval = 0.35f;
    constexpr float TargetMoveRepathDistance = 150.0f;
    constexpr float TargetMoveRepathDistanceSq = TargetMoveRepathDistance * TargetMoveRepathDistance;

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

        return SE::Math::Vector3{
            v.x * c - v.y * s,
            v.x * s + v.y * c,
            0.0f
        };
    }
}


BT::PortsList CatMoveToCannonRangeNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn)
    };
}

BT::NodeStatus CatMoveToCannonRangeNode::onStart()
{
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;

    elapsedRepath_ = RepathInterval;
    moveGoal_ = {};
    lastTargetPos_ = {};

    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc_) || selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn_) || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead() || targetPawn_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    const auto selfPos = selfNpc_->GetPosition();
    const auto targetPos = targetPawn_->GetPosition();

    const auto toTarget = targetPos - selfPos;
    const auto dirToTarget = Normalize2D(toTarget);

    // 처음에는 현재 위치 기준으로 좌/우 중 하나를 고정.
    // 랜덤으로 해도 되고, NPC id 기반으로 나눠도 됨.
    orbitSide_ = (selfNpc_->GetId().value % 2 == 0) ? 1 : -1;

    lastTargetPos_ = targetPos;

    return onRunning();
}

BT::NodeStatus CatMoveToCannonRangeNode::onRunning()
{
    if (selfNpc_ == nullptr || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead() || targetPawn_->IsDead()) {
        selfNpc_->StopMove();
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    const ServerMap& map = room->GetGameDataManager()->GetServerMap();

    const SE::Math::Vector3 selfPos = selfNpc_->GetPosition();
    const SE::Math::Vector3 targetPos = targetPawn_->GetPosition();
    const SE::Math::Vector3 targetFootPos = targetPos - SE::Math::Vector3{0.0f, 0.0f, 90.0f};

    const float dt = room->GetDelta();
    elapsedRepath_ += dt;

    const SE::Math::Vector3 toTarget = targetFootPos - selfPos;
    const float distSq = toTarget.LengthSq();

    const bool goodDistance =
    distSq >= CannonMinDistance * CannonMinDistance &&
    distSq <= CannonMaxDistance * CannonMaxDistance;
    
    // 이미 포격 가능한 거리면 이 노드는 성공.
    if (goodDistance &&
        CanShootTarget(selfNpc_, targetPawn_))
    {
        selfNpc_->StopMove();
        return BT::NodeStatus::SUCCESS;
    }

    const bool targetMovedEnough =
        (targetPos - lastTargetPos_).LengthSq() >= TargetMoveRepathDistanceSq;

    const bool needRepath =
        elapsedRepath_ >= RepathInterval || targetMovedEnough;

    if (needRepath)
    {
        elapsedRepath_ = 0.0f;
        lastTargetPos_ = targetPos;

        const SE::Math::Vector3 targetToSelf = Normalize2D(selfPos - targetPos);

        // 직선 후퇴 지점이 아니라, 좌/우로 비튼 사냥 위치.
        // 60도 정도 비틀면 원을 그리며 붙는 느낌이 남.
        const float orbitAngle = orbitSide_ * 60.0f * Pi / 180.0f;

        SE::Math::Vector3 orbitDir = Rotate2D(targetToSelf, orbitAngle);

        SE::Math::Vector3 desiredPos =
            targetPos + orbitDir * CannonDesiredDistance;

        desiredPos.z = targetPos.z;

        SE::Math::Vector3 navPos{};
        if (!map.ProjectToNavMesh(desiredPos, navPos))
        {
            // 한쪽이 막혔으면 반대쪽도 시도
            orbitSide_ *= -1;

            orbitDir = Rotate2D(targetToSelf, -orbitAngle);
            desiredPos = targetPos + orbitDir * CannonDesiredDistance;
            desiredPos.z = targetPos.z;

            if (!map.ProjectToNavMesh(desiredPos, navPos)) {
                return BT::NodeStatus::FAILURE;
            }
        }

        std::vector<SE::Math::Vector3> path;
        if (map.FindPath(selfPos, navPos, path) != NavPathResult::Success)
        {
            orbitSide_ *= -1;

            orbitDir = Rotate2D(targetToSelf, -orbitAngle);
            desiredPos = targetPos + orbitDir * CannonDesiredDistance;
            desiredPos.z = targetPos.z;

            if (!map.ProjectToNavMesh(desiredPos, navPos)) {
                return BT::NodeStatus::FAILURE;
            }

            path.clear();
            if (map.FindPath(selfPos, navPos, path) != NavPathResult::Success) {
                return BT::NodeStatus::FAILURE;
            }
        }

        moveGoal_ = navPos;
        selfNpc_->MoveAlongPath(std::move(path), ArriveDistance);
    }

    if ((moveGoal_ - selfPos).LengthSq() <= ArriveDistanceSq)
    {
        selfNpc_->StopMove();
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::RUNNING;
}

void CatMoveToCannonRangeNode::onHalted()
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

bool CatMoveToCannonRangeNode::CanShootTarget(MonsterPawn* selfPawn, Pawn* targetPawn)
{
    auto room = selfPawn->GetRoom();
    if (room == nullptr) {
        return false;
    }

    constexpr float CannonRange = 2000.0f;

    const SE::Math::Vector3 origin =
        selfPawn->GetPosition() + SE::Math::Vector3{0.0f, 0.0f, 90.0f};

    const SE::Math::Vector3 target =
        targetPawn->GetPosition() + SE::Math::Vector3{0.0f, 0.0f, 90.0f};

    SE::Math::Vector3 dir = target - origin;
    if (dir.LengthSq() <= 0.0001f) {
        return false;
    }

    dir = dir.Normalized();

    SE::Physics::Ray ray(origin, dir, CannonRange);

    return room->GetRoomGameSystem()
        .GetCombatSystem()
        .CanSeeTarget(ray);
}

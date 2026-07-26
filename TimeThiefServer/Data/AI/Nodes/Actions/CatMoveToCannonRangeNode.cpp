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
    constexpr float CannonFireRange = 2000.0f;
    constexpr float CannonFireRangeSq = CannonFireRange * CannonFireRange;
    constexpr SE::Math::Vector3 CannonAimOffset{112.150f, 6.806f, 42.250f};

    constexpr float ArriveDistance = 120.0f;
    constexpr float ArriveDistanceSq = ArriveDistance * ArriveDistance;

    constexpr float RepathInterval = 0.35f;
    constexpr float TargetMoveRepathDistance = 150.0f;
    constexpr float TargetMoveRepathDistanceSq = TargetMoveRepathDistance * TargetMoveRepathDistance;
    constexpr float SideSearchAngle = 35.0f * Pi / 180.0f;
    constexpr float WideSideSearchAngle = 70.0f * Pi / 180.0f;

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
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
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

    const auto targetPos = targetPawn_->GetPosition();

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

    const SE::Math::Vector3 selfPos = selfNpc_->GetPosition();
    const SE::Math::Vector3 targetPos = targetPawn_->GetPosition();

    const float dt = room->GetDelta();
    elapsedRepath_ += dt;

    const SE::Math::Vector3 toTarget = targetPos - selfPos;
    const float distSq = toTarget.LengthSq();

    // 이미 포격 가능한 거리면 이 노드는 성공.
    if (distSq <= CannonFireRangeSq &&
        CanShootTarget(selfNpc_, targetPawn_))
    {
        selfNpc_->StopMove();
        return BT::NodeStatus::SUCCESS;
    }

    // 이미 원하는 포격 거리보다 가까운데 시야가 없다면 뒤로 물러나서
    // 거리를 유지하지 않는다. 원거리 모드를 포기하고 다음 판단에서
    // 근접 공격을 선택할 수 있도록 한다.
    if (distSq <= CannonDesiredDistance * CannonDesiredDistance)
    {
        selfNpc_->StopMove();
        SwitchToMeleeMode();
        return BT::NodeStatus::FAILURE;
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

        // 원거리 모드는 플레이어 주변을 계속 도는 대신, 현재 방위의 사격 거리로 안정적으로 이동한다.
        // 직선 지점이 막힌 경우에만 좌/우 후보를 제한적으로 시도한다.
        const SE::Math::Vector3 directGoal = targetPos + targetToSelf * CannonDesiredDistance;
        const SE::Math::Vector3 sideGoal =
            targetPos + Rotate2D(targetToSelf, orbitSide_ * SideSearchAngle) * CannonDesiredDistance;
        const SE::Math::Vector3 oppositeSideGoal =
            targetPos + Rotate2D(targetToSelf, -orbitSide_ * SideSearchAngle) * CannonDesiredDistance;
        const SE::Math::Vector3 wideSideGoal =
            targetPos + Rotate2D(targetToSelf, orbitSide_ * WideSideSearchAngle) * CannonDesiredDistance;
        const SE::Math::Vector3 oppositeWideSideGoal =
            targetPos + Rotate2D(targetToSelf, -orbitSide_ * WideSideSearchAngle) * CannonDesiredDistance;

        if (!TryMoveToGoal(selfPos, directGoal) &&
            !TryMoveToGoal(selfPos, sideGoal) &&
            !TryMoveToGoal(selfPos, oppositeSideGoal) &&
            !TryMoveToGoal(selfPos, wideSideGoal) &&
            !TryMoveToGoal(selfPos, oppositeWideSideGoal))
        {
            if (distSq <= CannonFireRangeSq && CanShootTarget(selfNpc_, targetPawn_)) {
                selfNpc_->StopMove();
                return BT::NodeStatus::SUCCESS;
            }

            selfNpc_->StopMove();
            SwitchToMeleeMode();
            return BT::NodeStatus::FAILURE;
        }
    }

    if ((moveGoal_ - selfPos).LengthSq() <= ArriveDistanceSq)
    {
        selfNpc_->StopMove();

        if (distSq <= CannonFireRangeSq &&
            CanShootTarget(selfNpc_, targetPawn_))
        {
            return BT::NodeStatus::SUCCESS;
        }

        // 이동 목표에 도착했어도 사격할 수 없다면 성공으로 처리하지 않는다.
        // 원거리 모드를 해제해야 다음 틱에 근접 행동을 다시 선택할 수 있다.
        SwitchToMeleeMode();
        return BT::NodeStatus::FAILURE;
    }

    return BT::NodeStatus::RUNNING;
}

void CatMoveToCannonRangeNode::SwitchToMeleeMode()
{
    setOutput<CombatEventType>(BB::CombatMode, CombatEventType::CatMelee);
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

    const SE::Math::Vector3 origin =
        selfPawn->TransformLocalOffsetToWorld(CannonAimOffset);

    const SE::Math::Vector3 target =
        targetPawn->GetPosition();

    SE::Math::Vector3 dir = target - origin;
    const float targetDistance = dir.Length();
    if (targetDistance <= 0.0001f) {
        return false;
    }

    // 사거리 자체는 호출부에서 검사한다. 여기서는 타깃 뒤의 지형이 아니라
    // Cat과 타깃 사이에 있는 지형만 시야 차단물로 취급한다.
    constexpr float TargetEndpointEpsilon = 1.0f;
    dir /= targetDistance;

    SE::Physics::Ray ray(
        origin,
        dir,
        std::max(0.0f, targetDistance - TargetEndpointEpsilon));

    return room->GetRoomGameSystem()
        .GetCombatSystem()
        .CanSeeTarget(ray);
}

bool CatMoveToCannonRangeNode::TryMoveToGoal(const SE::Math::Vector3& selfPos, const SE::Math::Vector3& goal)
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

    SE::Math::Vector3 desiredGoal = goal;
    desiredGoal.z = goal.z;

    SE::Math::Vector3 navGoal{};
    if (!map.ProjectToNavMesh(*navQueryContext, desiredGoal, navGoal)) {
        return false;
    }

    std::vector<SE::Math::Vector3> path;
    if (map.FindPath(*navQueryContext, selfPos, navGoal, path) != NavPathResult::Success || path.empty()) {
        return false;
    }

    moveGoal_ = navGoal;
    selfNpc_->MoveAlongPath(std::move(path), ArriveDistance);
    return true;
}

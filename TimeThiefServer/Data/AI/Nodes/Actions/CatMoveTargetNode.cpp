#include "pch.h"
#include "CatMoveTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Data/GameDataManager.h"
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
    constexpr float DirectApproachDistance = 120.0f;
    
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
        BT::BidirectionalPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
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
        
        if (selfNpc_) {
            selfNpc_->ClearTarget();
        }
        
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

        // 정면으로 꽂히지 않도록 살짝 좌/우로 비틀어서 접근한다.
        // 후보 지점이 NavMesh 밖이면 MoveTo 내부에서 StopMove만 되고 이 노드가
        // RUNNING에 머무를 수 있으므로, 여기서 경로 생성까지 검증한다.
        const SE::Math::Vector3 primaryDir =
            Rotate2D(targetToSelf, orbitSide_ * ApproachSideAngle);
        const SE::Math::Vector3 secondaryDir =
            Rotate2D(targetToSelf, -orbitSide_ * ApproachSideAngle);

        if (!TryMoveToGoal(selfPos, targetPos + primaryDir * ApproachDistance) &&
            !TryMoveToGoal(selfPos, targetPos + secondaryDir * ApproachDistance) &&
            !TryMoveToGoal(selfPos, targetPos + targetToSelf * DirectApproachDistance))
        {
            selfNpc_->StopMove();
            setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
            orbitSide_ *= -1;
            return BT::NodeStatus::FAILURE;
        }
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

bool CatMoveTargetNode::TryMoveToGoal(const SE::Math::Vector3& selfPos, const SE::Math::Vector3& goal)
{
    if (selfNpc_ == nullptr) {
        return false;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr || room->GetGameDataManager() == nullptr) {
        return false;
    }

    const ServerMap& map = room->GetGameDataManager()->GetServerMap();

    SE::Math::Vector3 desiredGoal = goal;
    desiredGoal.z = goal.z;

    SE::Math::Vector3 navGoal{};
    if (!map.ProjectToNavMesh(desiredGoal, navGoal)) {
        return false;
    }

    std::vector<SE::Math::Vector3> path;
    if (map.FindPath(selfPos, navGoal, path) != NavPathResult::Success || path.empty()) {
        return false;
    }

    moveGoal_ = navGoal;
    selfNpc_->MoveAlongPath(std::move(path), MoveArriveDistance);
    return true;
}

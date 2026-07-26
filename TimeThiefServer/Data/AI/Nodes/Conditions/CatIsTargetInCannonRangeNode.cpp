#include "pch.h"
#include "CatIsTargetInCannonRangeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Physics/Ray/Ray.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr SE::Math::Vector3 CannonAimOffset{112.150f, 6.806f, 42.250f};
}


BT::PortsList CatIsTargetInCannonRangeNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn)
    };
}

BT::NodeStatus CatIsTargetInCannonRangeNode::tick()
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

    auto room = selfNpc->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    auto& gameSystem = room->GetRoomGameSystem();
    CombatSystem& combatSystem = gameSystem.GetCombatSystem();

    constexpr float CannonRange = 2000.0f;
    constexpr float CannonRangeSq = CannonRange * CannonRange;

    const SE::Math::Vector3 selfPos = selfNpc->GetPosition();

    const SE::Math::Vector3 targetPos = targetPawn->GetPosition();

    const float distSq = (targetPos - selfPos).LengthSq();

    if (distSq > CannonRangeSq) {
        return BT::NodeStatus::FAILURE;
    }

    const SE::Math::Vector3 sightOrigin =
        selfNpc->TransformLocalOffsetToWorld(CannonAimOffset);

    const SE::Math::Vector3 toTarget = targetPos - sightOrigin;
    const float targetDistance = toTarget.Length();
    if (targetDistance <= 0.0001f) {
        return BT::NodeStatus::FAILURE;
    }

    // 타깃 뒤쪽의 벽을 시야 차단물로 오인하지 않도록 실제 타깃까지의 구간만 검사한다.
    constexpr float TargetEndpointEpsilon = 1.0f;
    const SE::Math::Vector3 dir = toTarget / targetDistance;
    SE::Physics::Ray ray(
        sightOrigin,
        dir,
        std::max(0.0f, targetDistance - TargetEndpointEpsilon));

    if (!combatSystem.CanSeeTarget(ray)) {
        return BT::NodeStatus::FAILURE;
    }

    return BT::NodeStatus::SUCCESS;
}

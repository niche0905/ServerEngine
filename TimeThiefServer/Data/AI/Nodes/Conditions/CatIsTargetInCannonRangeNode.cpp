#include "pch.h"
#include "CatIsTargetInCannonRangeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Physics/Ray/Ray.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;


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

    const SE::Math::Vector3 selfPos =
        selfNpc->GetPosition();     // Muzzle offset 적용

    const SE::Math::Vector3 targetPos =
        targetPawn->GetPosition() - SE::Math::Vector3{0.0f, 0.0f, 90.0f};

    const float distSq = (targetPos - selfPos).LengthSq();

    if (distSq > CannonRangeSq) {
        return BT::NodeStatus::FAILURE;
    }

    const SE::Math::Vector3 dir = (targetPos - selfPos).Normalized();
    if (dir == SE::Math::Vector3::Zero()) {
        return BT::NodeStatus::FAILURE;
    }
    
    SE::Physics::Ray ray(selfPos, dir, CannonRange + 300.0f);
    if (!combatSystem.CanSeeTarget(ray)) {
        return BT::NodeStatus::FAILURE;
    }

    return BT::NodeStatus::SUCCESS;
}

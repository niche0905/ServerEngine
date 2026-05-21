#include "pch.h"
#include "CatDecideCombatModeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Data/GameDataManager.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace BBH = AiBlackboard;


BT::PortsList CatDecideCombatModeNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::BidirectionalPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatDecideCombatModeNode::tick()
{
    CombatEventType mode = CombatEventType::None;

    if (getInput<CombatEventType>(BB::CombatMode, mode)) {
        if (mode != CombatEventType::None) {
            return BT::NodeStatus::SUCCESS;
        }
    }
    
    Pawn* targetPawn = nullptr;
    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn) or targetPawn == nullptr) {
        return BT::NodeStatus::FAILURE;
    }
    
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) or selfNpc == nullptr) {
        return BT::NodeStatus::FAILURE;
    }
    
    auto room = selfNpc->GetRoom();
    if (room == nullptr)
        return BT::NodeStatus::FAILURE;
    
    auto* gameDataManager = room->GetGameDataManager();
    if (gameDataManager == nullptr)
        return BT::NodeStatus::FAILURE;
    
    const ServerMap& map = gameDataManager->GetServerMap();
    
    const auto& selfPos = selfNpc->GetPosition();
    const auto& targetPos = targetPawn->GetPosition() - SE::Math::Vector3{0.0f, 0.0f, 90.0f};   // TEMP
    
    const bool isReachable = map.IsReachablePosition(selfPos, targetPos, SE::Math::Vector3{200.0f, 200.0f, 300.0f});
    
    if (not isReachable) {
        setOutput<CombatEventType>(BB::CombatMode, CombatEventType::CatRange);
    }
    else {
        if (BBH::RandomChance(0.3f)) {  // 30% 확률로 근접 공격
            setOutput<CombatEventType>(BB::CombatMode, CombatEventType::CatRange);
        }
        else {  // 70% 확률로 원거리 공격
            setOutput<CombatEventType>(BB::CombatMode, CombatEventType::CatMelee);
        }
    }
    
    return BT::NodeStatus::SUCCESS;
}

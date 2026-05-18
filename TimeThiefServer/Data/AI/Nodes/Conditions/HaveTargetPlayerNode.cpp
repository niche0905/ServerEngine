#include "pch.h"
#include "HaveTargetPlayerNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace BBT = AiBlackboard;


BT::PortsList HaveTargetPlayerNode::providedPorts()
{
    return { 
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<ObjectId>(BB::TargetId), 
        BT::OutputPort<Pawn*>(BB::TargetPawn) 
    };
}

BT::NodeStatus HaveTargetPlayerNode::tick()
{
    Pawn* targetPawn = nullptr;
    if (getInput<Pawn*>(BB::TargetPawn, targetPawn) and targetPawn != nullptr) {
        if (targetPawn->IsDead())
            return BT::NodeStatus::FAILURE;
        
        return BT::NodeStatus::SUCCESS;
    }
    
    ObjectId targetId;
    if (!getInput<ObjectId>(BB::TargetId, targetId)) {
        return BT::NodeStatus::FAILURE;
    }
    
    if (targetId == ObjectId{}) {
        return BT::NodeStatus::FAILURE;
    }
    
    MonsterPawn* selfNpc = nullptr;
    if (getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) and selfNpc != nullptr) {
        return BT::NodeStatus::FAILURE;
    }
    
    auto room = selfNpc->GetRoom();
    if (room == nullptr)
        return BT::NodeStatus::FAILURE;
    
    Pawn* foundTarget = room->GetObjectManager().FindAs<Pawn>(targetId);
    if (foundTarget == nullptr or foundTarget->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }
    
    if (foundTarget->IsDead())
        return BT::NodeStatus::FAILURE;
    
    // Blackboard에 캐싱
    setOutput<Pawn*>(BB::TargetPawn, foundTarget);
    
    return BT::NodeStatus::SUCCESS;
}

#include "pch.h"
#include "HaveTargetPlayerNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace BBT = AiBlackboard;


BT::NodeStatus HaveTargetPlayerNode::tick()
{
    auto blackboard = config().blackboard;
    if (blackboard == nullptr)
        return BT::NodeStatus::FAILURE;
    
    Pawn* targetPawn = nullptr;
    if (blackboard->get(BB::TargetPawn, targetPawn) and targetPawn != nullptr) {
        if (targetPawn->IsDead())
            return BT::NodeStatus::FAILURE;
        
        return BT::NodeStatus::SUCCESS;
    }
    
    ObjectId targetId;
    if (!blackboard->get(BB::TargetId, targetId)) {
        return BT::NodeStatus::FAILURE;
    }
    
    if (targetId == ObjectId{}) {
        return BT::NodeStatus::FAILURE;
    }
    
    MonsterPawn* selfNpc = BBT::GetSelfNpc(blackboard);
    if (selfNpc == nullptr) {
        return BT::NodeStatus::FAILURE;
    }
    
    auto room = selfNpc->GetRoom();
    if (room == nullptr)
        return BT::NodeStatus::FAILURE;
    
    Pawn* foundTarget = room->GetObjectManager().FindAs<Pawn>(targetId);
    if (foundTarget == nullptr) {
        return BT::NodeStatus::FAILURE;
    }
    
    if (foundTarget->IsDead())
        return BT::NodeStatus::FAILURE;
    
    // Blackboard에 캐싱
    blackboard->set(BB::TargetPawn, targetPawn);
    
    return BT::NodeStatus::SUCCESS;
}

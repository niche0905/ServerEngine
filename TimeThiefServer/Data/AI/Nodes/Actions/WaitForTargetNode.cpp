#include "pch.h"
#include "WaitForTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace BBT = AiBlackboard;


BT::PortsList WaitForTargetNode::providedPorts()
{
    return { 
        BT::InputPort<MonsterPawn*>(BB::SelfNpc), 
        BT::OutputPort<Pawn*>(BB::TargetPawn), 
        BT::OutputPort<ObjectId>(BB::TargetId) 
    };
}

BT::NodeStatus WaitForTargetNode::onStart()
{
    return onRunning();
}

BT::NodeStatus WaitForTargetNode::onRunning()
{
    // consoleLogger->Log(Color::Blue, L"WaitForTargetNode ticked.\n");
    
    MonsterPawn* selfNpc = BBT::GetSelfNpc(config().blackboard);
    if (selfNpc == nullptr) {
        return BT::NodeStatus::FAILURE;
    }
    
    auto room = selfNpc->GetRoom();
    if (room == nullptr)
        return BT::NodeStatus::FAILURE;
    
    const SE::Math::Vector3& selfPos = selfNpc->GetPosition();
    
    const float DETECT_DISTANCE = 1000.0f;
    const float DETECT_DISTANCE_SQ = DETECT_DISTANCE * DETECT_DISTANCE;
    
    PlayerPawn* nearestTarget = nullptr;
    float nearestDistSq = DETECT_DISTANCE_SQ;
    
    room->GetObjectManager().ForEachAlive([&](BaseObject* object)
    {
        auto* playerPawn = dynamic_cast<PlayerPawn*>(object);
        if (playerPawn == nullptr)
            return;
        
        const SE::Math::Vector3& targetPos = playerPawn->GetPosition();
        const float distSq = (targetPos - selfPos).LengthSq();
        
        if (distSq > DETECT_DISTANCE_SQ)
            return;
        
        if (nearestTarget == nullptr or distSq < nearestDistSq) {
            nearestTarget = playerPawn;
            nearestDistSq = distSq;
        }
    });
    
    if (nearestTarget == nullptr) {
        setOutput<Pawn*>(BB::TargetPawn, nullptr);
        setOutput<ObjectId>(BB::TargetId, ObjectId{});
        return BT::NodeStatus::RUNNING;   // 계속 대기
    }
    
    // 타겟 발견, Blackboard에 저장
    Pawn* targetPawn = nearestTarget;
    setOutput<Pawn*>(BB::TargetPawn, targetPawn);
    setOutput<ObjectId>(BB::TargetId, nearestTarget->GetId());
    
    return BT::NodeStatus::SUCCESS;
}

void WaitForTargetNode::onHalted()
{
}

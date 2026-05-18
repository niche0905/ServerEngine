#include "pch.h"
#include "MoveTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Data/GameDataManager.h"
#include "Data/Map/ServerMap.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace BBT = AiBlackboard;


BT::PortsList MoveTargetNode::providedPorts()
{
    return { 
        BT::InputPort<MonsterPawn*>(BB::SelfNpc), 
        BT::InputPort<Pawn*>(BB::TargetPawn) 
    };
}

BT::NodeStatus MoveTargetNode::onStart()
{
    return onRunning();
}

BT::NodeStatus MoveTargetNode::onRunning()
{
    // consoleLogger->Log(Color::Blue, L"MoveTargetNode ticked.\n");
    
    MonsterPawn* selfNpc = BBT::GetSelfNpc(config().blackboard);
    if (selfNpc == nullptr) {
        return BT::NodeStatus::FAILURE;
    }
    
    auto room = selfNpc->GetRoom();
    if (room == nullptr)
        return BT::NodeStatus::FAILURE;
    
    Pawn* targetPawn = nullptr;
    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn) or targetPawn == nullptr) {
        return BT::NodeStatus::FAILURE;
    }
    
    const auto& selfPos = selfNpc->GetPosition();
    const auto& targetPos = targetPawn->GetPosition();
    
    constexpr float ARRIVE_DISTANCE = 100.0f;
    constexpr float ARRIVE_DISTANCE_SQ = ARRIVE_DISTANCE * ARRIVE_DISTANCE;
    
    if ((targetPos - selfPos).LengthSq() <= ARRIVE_DISTANCE_SQ) {
        return BT::NodeStatus::SUCCESS;   // 이미 충분히 가까이 도착한 경우
    }
    
    const ServerMap& map = room->GetGameDataManager()->GetServerMap();
    
    std::vector<SE::Math::Vector3> path;
    if (!map.FindPath(selfPos, targetPos, path))
        return BT::NodeStatus::FAILURE;
    
    if (path.empty())
        return BT::NodeStatus::FAILURE;
    
    // path[0]은 너무 가까울 수 있어서 path.front() 대신 path[1]을 목표 지점으로 삼는 것도 고려할 수 있음
    SE::Math::Vector3 nextMovePos = path.front();
    if (path.size() >= 2)
        nextMovePos = path[1];
    
    selfNpc->MoveTo(nextMovePos);
    
    return BT::NodeStatus::RUNNING;
}

void MoveTargetNode::onHalted()
{
    MonsterPawn* selfNpc = BBT::GetSelfNpc(config().blackboard);
    if (selfNpc == nullptr) {
        return;
    }
    
    selfNpc->StopMove();
}


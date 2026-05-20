#include "pch.h"
#include "MoveTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Data/GameDataManager.h"
#include "Data/Map/ServerMap.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;


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
    using namespace SE::Math;
    
    // consoleLogger->Log(Color::Blue, L"MoveTargetNode ticked.\n");
    
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) or selfNpc == nullptr) {
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
    const auto& targetPos = targetPawn->GetPosition() - SE::Math::Vector3{0.0f, 0.0f, 90.0f};   // TEMP
    
    constexpr float ARRIVE_DISTANCE = 100.0f;
    constexpr float ARRIVE_DISTANCE_SQ = ARRIVE_DISTANCE * ARRIVE_DISTANCE;
    
    if ((targetPos - selfPos).Length2DSq() <= ARRIVE_DISTANCE_SQ) {
        selfNpc->StopMove();
        return BT::NodeStatus::SUCCESS;   // 이미 충분히 가까이 도착한 경우
    }
    
    const ServerMap& map = room->GetGameDataManager()->GetServerMap();
    
    std::vector<Vector3> path;
    NavPathResult result = map.FindPath(selfPos, targetPos, path);
    
    if (result != NavPathResult::Success || path.size() < 2) {
        selfNpc->StopMove();
        return BT::NodeStatus::RUNNING;
    }
    
    Vector3 next = path[1];
    next.z = selfNpc->GetPosition().z;
    selfNpc->MoveTo(next);
    
    return BT::NodeStatus::RUNNING;
}

void MoveTargetNode::onHalted()
{
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) or selfNpc == nullptr) {
        return;
    }
    
    selfNpc->StopMove();
}


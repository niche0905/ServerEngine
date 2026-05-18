#pragma once
#include <behaviortree_cpp/bt_factory.h>

class NpcAiTable;

/*-------------
   AiManager
-------------*/
//
// AiManager는 게임 내 NPC의 인공지능을 관리하는 클래스입니다.
// (매 인스턴스가 아닌, factory를 통해 NPC마다 AIController를 생성하는 형태로 구현할 예정입니다.)
//

class AiManager
{
public:
   bool Init(const NpcAiTable& table);
   
   BT::Tree CreateTree(uint32 npcId);
   
private:
   BT::BehaviorTreeFactory factory_;
   const NpcAiTable* table_ = nullptr;   // non-owning
    
};

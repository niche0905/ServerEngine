#include "pch.h"
#include "AiManager.h"
#include "Data/Tables/NpcAiTable.h"

//////////////////////////////////////////////////////////
/// Test Codes
//////////////////////////////////////////////////////////

class CheckEnemy : public BT::ConditionNode
{
public:
   CheckEnemy(const std::string& name, const BT::NodeConfiguration& config)
       : ConditionNode(name, config) {}

   static BT::PortsList providedPorts()
   {
      return {};
   }
   
   BT::NodeStatus tick() override
   {
      bool hasEnemy = true; // TODO: 실제 로직
      return hasEnemy ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
   }
};

class IsPlayerInRange : public BT::ConditionNode
{
public:
   IsPlayerInRange(const std::string& name, const BT::NodeConfiguration& config)
      :ConditionNode(name, config) {}

   static BT::PortsList providedPorts()
   {
      return {};
   }
   
   BT::NodeStatus tick() override
   {
      bool hasPlayer = true;
      return hasPlayer ? BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
   }
};

class Attack : public BT::SyncActionNode
{
public:
   Attack(const std::string& name, const BT::NodeConfiguration& config)
      : SyncActionNode(name, config) {}
   
   static BT::PortsList providedPorts()
   {
      return {};
   }
   
   BT::NodeStatus tick() override
   {
      consoleLogger->Log(Color::Magenta, L"Attack action executed.\n");
      return BT::NodeStatus::SUCCESS;
   }
};

class MoveRandom : public BT::SyncActionNode
{
public:
   MoveRandom(const std::string& name, const BT::NodeConfiguration& config)
      : SyncActionNode(name, config) {}
   
   static BT::PortsList providedPorts()
   {
      return {};
   }
   
   BT::NodeStatus tick() override
   {
      consoleLogger->Log(Color::Cyan, L"MoveRandom action executed.\n");
      return BT::NodeStatus::SUCCESS;
   }
};


//////////////////////////////////////////////////////////
/// end of Test Codes
//////////////////////////////////////////////////////////


/*-------------
   AiManager
-------------*/

bool AiManager::Init(const NpcAiTable& table)
{
   table_ = &table;
   
   factory_.registerNodeType<IsPlayerInRange>("IsPlayerInRange");
   factory_.registerNodeType<Attack>("Attack");
   factory_.registerNodeType<MoveRandom>("MoveRandom");
   
   for (const auto& [id, entry] : table.GetAll()) {
      try
      {
         factory_.registerBehaviorTreeFromFile(entry.btXmlPath);     // 실패하면 crash
      }
      catch (const std::exception& e)
      {
         consoleLogger->Log(Color::Red, L"[FATAL] BT Load Failed: %s\n", entry.btXmlPath.c_str());
         
         assert(false && "BehaviorTree load failed");
      }
   }
   
   return true;
}

BT::Tree AiManager::CreateTree(uint32 npcId)
{
   const auto* entry = table_->Find(npcId);
   if (!entry)
      throw std::runtime_error("NPC AI not found");
   
   return factory_.createTree(entry->btId);
}

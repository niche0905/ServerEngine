#include "pch.h"
#include "AiManager.h"
#include "Data/Tables/NpcAiTable.h"
#include "Nodes/AiNodeRegistry.h"

/*-------------
   AiManager
-------------*/

bool AiManager::Init(const NpcAiTable& table)
{
   table_ = &table;
   
   AiNodeRegistry::RegisterAll(factory_);
   
   for (const auto& [id, entry] : table.GetAll()) {
      try
      {
         factory_.registerBehaviorTreeFromFile(entry.btXmlPath);     // 실패하면 crash
      }
      catch (const std::exception& e)
      {
         consoleLogger->Log(Color::Red, L"[FATAL] BT Load Failed: %s\n", entry.btXmlPath.c_str());
         
         assert(false && "BehaviorTree load failed");
         return false;
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

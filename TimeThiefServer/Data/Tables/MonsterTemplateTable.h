#pragma once
#include <string>
#include <unordered_map>

struct MonsterTemplateDef
{
   uint32 templateId = 0;
   std::string name;
   int32 maxHp = 0;
   int32 dropPoint = 0;
   int32 respawnTimeSec = 0;
   int32 lootTableId = 0;

   bool IsValid() const
   {
      return templateId > 0
         && !name.empty()
         && maxHp > 0
         && dropPoint >= 0
         && respawnTimeSec >= 0
         && lootTableId > 0;
   }
};

struct MonsterTemplateTable
{
   std::unordered_map<uint32, MonsterTemplateDef> templates;

   const MonsterTemplateDef* GetTemplate(uint32 templateId) const
   {
      auto it = templates.find(templateId);
      if (it == templates.end())
         return nullptr;

      return &it->second;
   }

   bool HasTemplate(uint32 templateId) const
   {
      return templates.contains(templateId);
   }

   bool IsValid() const
   {
      for (const auto& [_, monsterTemplate] : templates) {
         if (!monsterTemplate.IsValid())
            return false;
      }

      return true;
   }
};

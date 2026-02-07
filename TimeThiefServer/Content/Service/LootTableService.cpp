#include "pch.h"
#include "LootTableService.h"
#include "Utils/Random/WeightedRandom.h"
#include <json/json.h>

namespace 
{
   inline bool Has(const Json::Value& v, const char* key)
   {
      return v.isObject() and v.isMember(key);
   }
   
   inline int32 AsI32(const Json::Value& v, int32 def = 0)
   {
      return v.isInt() ? v.asInt() : def;
   }
   
   inline uint32 AsU32(const Json::Value& v, uint32 def = 0)
   {
      return v.isUInt() ? v.asUInt() : def;
   }
   
   inline int64 AsI64(const Json::Value& v, int64 def = 0)
   {
      return v.isInt64() ? v.asInt64() : def;
   }
   
   inline double AsF64(const Json::Value& v, double def = 0.0)
   {
      return v.isDouble() ? v.asDouble() : (v.isInt() ? static_cast<double>(v.asInt()) : def);
      
      // THINK: 아래로 변경...??
      // if (v.isDouble()) return v.asDouble();
      // if (v.isInt()) return static_cast<double>(v.asInt());
      // if (v.isInt64()) return static_cast<double>(v.asInt64());
      // if (v.isUInt()) return static_cast<double>(v.asUInt());
      // if (v.isUInt64()) return static_cast<double>(v.asUInt64());
      // return def;
   }
   
   inline std::string AsString(const Json::Value& v, std::string def = {})
   {
      return v.isString() ? v.asString() : def;
   }
   
   inline bool AsBool(const Json::Value& v, bool def = false)
   {
      return v.isBool() ? v.asBool() : def;
   }
   
   inline IntRange ReadIntRange(const Json::Value& obj, const char* key, int32 defMin, int32 defMax)
   {
      IntRange range{defMin, defMax};
      if (not Has(obj, key)) return range;
      
      const Json::Value& rr = obj[key];
      if (rr.isObject()) {
         if (Has(rr, "min"))
            range.min = AsI32(rr["min"], range.min);
         
         if (Has(rr, "max"))
            range.max = AsI32(rr["max"], range.max);
      }
      
      return range;
   }
   
   inline Int64Range ReadInt64Range(const Json::Value& obj, const char* key, int64 defMin, int64 defMax)
   {
      Int64Range range{defMin, defMax};
      if (not Has(obj, key)) return range;
      
      const Json::Value& rr = obj[key];
      if (rr.isObject()) {
         if (Has(rr, "min"))
            range.min = AsI64(rr["min"], range.min);
         
         if (Has(rr, "max"))
            range.max = AsI64(rr["max"], range.max);
      }
      
      return range;
   }
   
   inline Chance ReadChance(const Json::Value& obj, const char* key, double def = 1.0)
   {
      Chance chance{def};
      if (not Has(obj, key)) return chance;
      
      chance.value = AsF64(obj[key], def);
      return chance;
   }
   
   inline int64 RollRangeI64(int64 minInclusive, int64 maxInclusive, Random32& rng)
   {
      if (minInclusive >= maxInclusive)
         return minInclusive;
      const uint64 span = static_cast<uint64>(maxInclusive - minInclusive) + 1ull;
      
      // 범위가 32비트 이내인 경우
      if (span <= 0xFFFFFFFFull) {
         const uint32 r = rng.NextU32(static_cast<uint32>(span));
         return minInclusive + static_cast<int64>(r);
      }
      
      // 64비트 범위인 경우
      const uint64 r64 = (static_cast<uint64>(rng.NextU32()) << 32) | static_cast<uint64>(rng.NextU32());
      return minInclusive + static_cast<int64>(r64 % span);
   }
   
}

/*--------------------
   LootTableService
--------------------*/

bool LootTableService::LoadFromFile(const std::string& filepath)
{
   SE::Config::ConfigDocument doc;
   if (not configLoader_.LoadFromFile(filepath, doc)) {
      consoleLogger->Log(Color::Red, L"LootTableService::LoadFromFile - Failed to load loot table file '%S': %s", filepath.c_str(), configLoader_.GetLastError());
      return false;
   }
   
   const Json::Value& root = doc.Root();
   
   if (not root.isObject())
      return false;
   
   std::unordered_map<int32, LootTableDef> newTables;
   
   if (not Has(root, "tables"))
      return false;
   
   const Json::Value& tablesNode = root["tables"];
   
   auto parseOneTable = [&](int32 tableId, const Json::Value& tableObj) -> bool
   {
      LootTableDef def{};
      def.tableId = tableId;
      
      if (Has(tableObj, "groups") and tableObj["groups"].isArray()) {
         
         const Json::Value& groups = tableObj["groups"];
         def.groups.reserve(groups.size());
         
         for (Json::ArrayIndex gi = 0; gi < groups.size(); ++gi) {
            
            const Json::Value& gobj = groups[gi];
            if (not gobj.isObject()) continue;
            
            LootGroup g{};
            
            g.id = Has(gobj, "id") ? AsString(gobj["id"], "") : "";
            g.chance = ReadChance(gobj, "chance", 1.0);
            g.pickCount = ReadIntRange(gobj, "pick", 0, 0);
            g.allowDuplicates = Has(gobj, "allowDuplicates") ? AsBool(gobj["allowDuplicates"], false) : false;
            
            if (Has(gobj, "entries") and gobj["entries"].isArray()) {
               
               const Json::Value& entries = gobj["entries"];
               g.entries.reserve(entries.size());
               
               for (Json::ArrayIndex ei = 0; ei < entries.size(); ++ei) {
                  
                  const Json::Value& eobj = entries[ei];
                  if (not eobj.isObject()) continue;
                  
                  LootEntry entry{};
                  
                  entry.weight = Has(eobj, "weight") ? AsI32(eobj["weight"], 1) : 1;
                  entry.chance = ReadChance(eobj, "chance", 1.0);
                  
                  if (Has(eobj, "itemId")) {
                     entry.itemId = static_cast<ItemId>(AsU32(eobj["itemId"], 0));
                     entry.itemCount = ReadIntRange(eobj, "count", 1, 1);
                  }
                  else if (Has(eobj, "currencyId")) {
                     entry.currencyId = static_cast<CurrencyId>(AsU32(eobj["currencyId"], 0));
                     entry.moneyAmount = ReadInt64Range(eobj, "amount", 0, 0);
                  }
                  else {
                     continue;
                  }
                  
                  if (entry.IsValid())
                     g.entries.push_back(entry);
               }
            }
            
            if (g.IsValid())
               def.groups.push_back(std::move(g));
         }
      }
      
      if (not def.IsValid())
         return false;
      
      newTables.emplace(def.tableId, std::move(def));
      return true;
   };
   
   if (tablesNode.isObject()) {
      const auto keys = tablesNode.getMemberNames();
      for (const auto& key : keys) {
         const int32 tableId = std::stoi(key);
         const Json::Value& tableObj = tablesNode[key];
         if (not tableObj.isObject()) continue;
         
         if (not parseOneTable(tableId, tableObj)) {
            consoleLogger->Log(Color::Red, L"LootTableService::LoadFromFile - Failed to parse loot table id %d in file '%S'", tableId, filepath.c_str());
            return false;
         }
      }
   }
   else if (tablesNode.isArray()) {
      for (Json::ArrayIndex i = 0; i < tablesNode.size(); ++i) {
         const Json::Value& tableObj = tablesNode[i];
         if (not tableObj.isObject()) continue;
         if (not Has(tableObj, "tableId")) continue;
         
         const int32 tableId = AsI32(tableObj["tableId"], 0);
         if (tableId <= 0) continue;
         
         if (not parseOneTable(tableId, tableObj)) {
            consoleLogger->Log(Color::Red, L"LootTableService::LoadFromFile - Failed to parse loot table id %d in file '%S'", tableId, filepath.c_str());
            return false;
         }
      }
   }
   else {
      return false;
   }
   
   tables_.swap(newTables);
   lastPath = filepath;
   return true;
}

bool LootTableService::Reload()
{
   if (lastPath.empty()) return false;
   
   return LoadFromFile(lastPath);
}

LootBundle LootTableService::Roll(int32 tableId, uint32 rngSeed, const LootRollContext& ctx) const
{
   LootBundle out;
   
   auto it = tables_.find(tableId);
   if (it == tables_.end())
      return out;
   
   const LootTableDef& table = it->second;
   
   Random32 rng{rngSeed};
   
   std::vector<int32> picked;
   for (const auto& group : table.groups) {
      
      if (not rng.Chance(group.chance.value))
         continue;   // 그룹 출현 실패
      
      const int32 k = rng.NextI32(group.pickCount.min, group.pickCount.max);
      if (k <= 0)
         continue;   // 선택 개수 없음
      
      picked.clear();
      ChooseManyIndicesByWeight(static_cast<int32>(group.entries.size()), k, group.allowDuplicates, 
         [&](int32 i) { return group.entries[static_cast<size_t>(i)].weight; },
         rng, picked);
      
      for (int32 idx : picked) {
         if (idx < 0 or static_cast<size_t>(idx) >= group.entries.size())
            continue;
         
         const auto& entry = group.entries[static_cast<size_t>(idx)];
         
         if (not rng.Chance(entry.chance.value))
            continue;
         
         if (entry.IsItem()) {
            const int32 count = rng.NextI32(entry.itemCount.min, entry.itemCount.max);
            if (count > 0) {
               out.AddItem(entry.itemId, count);
            }
         }
         else if (entry.IsMoney()) {
            const int64 amount = RollRangeI64(entry.moneyAmount.min, entry.moneyAmount.max, rng);
            if (amount > 0) {
               out.AddMoney(entry.currencyId, amount);
            }
         }
      }
   }
   
   return out;
}

bool LootTableService::HasTable(int32 tableId) const
{
   return tables_.find(tableId) != tables_.end();
}

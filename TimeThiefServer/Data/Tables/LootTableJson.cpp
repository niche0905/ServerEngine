#include "pch.h"
#include "LootTableJson.h"
#include <fstream>
#include <json/json.h>

namespace 
{
    inline bool Has(const Json::Value& v, const char* key)
    {
        return v.isObject() && v.isMember(key);
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
        if (v.isDouble()) return v.asDouble();
        if (v.isInt())    return static_cast<double>(v.asInt());
        if (v.isInt64())  return static_cast<double>(v.asInt64());
        if (v.isUInt())   return static_cast<double>(v.asUInt());
        if (v.isUInt64()) return static_cast<double>(v.asUInt64());
        return def;
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
        IntRange range{ defMin, defMax };
        if (!Has(obj, key)) return range;

        const Json::Value& rr = obj[key];
        if (rr.isObject()) {
            if (Has(rr, "min")) range.min = AsI32(rr["min"], range.min);
            if (Has(rr, "max")) range.max = AsI32(rr["max"], range.max);
        }
        return range;
    }

    inline Int64Range ReadInt64Range(const Json::Value& obj, const char* key, int64 defMin, int64 defMax)
    {
        Int64Range range{ defMin, defMax };
        if (!Has(obj, key)) return range;

        const Json::Value& rr = obj[key];
        if (rr.isObject()) {
            if (Has(rr, "min")) range.min = AsI64(rr["min"], range.min);
            if (Has(rr, "max")) range.max = AsI64(rr["max"], range.max);
        }
        return range;
    }

    inline Chance ReadChance(const Json::Value& obj, const char* key, double def = 1.0)
    {
        Chance chance{ def };
        if (!Has(obj, key)) return chance;

        chance.value = AsF64(obj[key], def);
        return chance;
    }

    bool ParseOneTable(int32 tableId, const Json::Value& tableObj, LootTable& outTable)
    {
        LootTableDef def{};
        def.tableId = tableId;

        if (Has(tableObj, "groups") && tableObj["groups"].isArray()) {
            const Json::Value& groups = tableObj["groups"];
            def.groups.reserve(groups.size());

            for (Json::ArrayIndex gi = 0; gi < groups.size(); ++gi) {
                const Json::Value& gobj = groups[gi];
                if (!gobj.isObject())
                    continue;

                LootGroup g{};
                g.id = Has(gobj, "id") ? AsString(gobj["id"], "") : "";
                g.chance = ReadChance(gobj, "chance", 1.0);
                g.pickCount = ReadIntRange(gobj, "pick", 0, 0);
                g.allowDuplicates = Has(gobj, "allowDuplicates") ? AsBool(gobj["allowDuplicates"], false) : false;

                if (Has(gobj, "entries") && gobj["entries"].isArray()) {
                    const Json::Value& entries = gobj["entries"];
                    g.entries.reserve(entries.size());

                    for (Json::ArrayIndex ei = 0; ei < entries.size(); ++ei) {
                        const Json::Value& eobj = entries[ei];
                        if (!eobj.isObject())
                            continue;

                        LootEntry entry{};
                        entry.weight = Has(eobj, "weight") ? AsI32(eobj["weight"], 1) : 1;
                        entry.chance = ReadChance(eobj, "chance", 1.0);

                        if (Has(eobj, "itemId")) {
                            entry.itemId = static_cast<ItemId>(AsU32(eobj["itemId"], 0));
                            entry.itemCount = ReadIntRange(eobj, "count", 1, 1);
                        }
                        else if (Has(eobj, "currencyId")) {
                            entry.currencyId = static_cast<CurrencyType>(AsU32(eobj["currencyId"], 0));
                            entry.moneyAmount = ReadInt64Range(eobj, "amount", 0, 0);
                        }
                        else {
                            continue;
                        }

                        if (entry.IsValid())
                            g.entries.push_back(std::move(entry));
                    }
                }

                if (g.IsValid())
                    def.groups.push_back(std::move(g));
            }
        }

        if (!def.IsValid())
            return false;

        outTable.tables.emplace(def.tableId, std::move(def));
        return true;
    }
}

namespace LootTableJson
{
    bool LoadFromFile(const std::filesystem::path& filePath, LootTable& outTable, std::string* outError)
    {
        outTable.tables.clear();

        std::ifstream ifs(filePath);
        if (!ifs.is_open()) {
            if (outError) *outError = "Failed to open file";
            return false;
        }

        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;

        Json::Value root;
        std::string errs;
        if (!Json::parseFromStream(builder, ifs, &root, &errs)) {
            if (outError) *outError = errs;
            return false;
        }

        if (!root.isObject()) {
            if (outError) *outError = "Root must be object";
            return false;
        }

        if (!Has(root, "tables")) {
            if (outError) *outError = "Missing 'tables'";
            return false;
        }

        const Json::Value& tablesNode = root["tables"];
        LootTable newTable;

        if (tablesNode.isObject()) {
            const auto keys = tablesNode.getMemberNames();
            for (const auto& key : keys) {
                int32 tableId = 0;
                try {
                    tableId = std::stoi(key);
                }
                catch (...) {
                    if (outError) *outError = "Invalid table key: " + key;
                    return false;
                }

                const Json::Value& tableObj = tablesNode[key];
                if (!tableObj.isObject())
                    continue;

                if (!ParseOneTable(tableId, tableObj, newTable)) {
                    if (outError) *outError = "Failed to parse table id: " + std::to_string(tableId);
                    return false;
                }
            }
        }
        else if (tablesNode.isArray()) {
            for (Json::ArrayIndex i = 0; i < tablesNode.size(); ++i) {
                const Json::Value& tableObj = tablesNode[i];
                if (!tableObj.isObject())
                    continue;
                if (!Has(tableObj, "tableId"))
                    continue;

                const int32 tableId = AsI32(tableObj["tableId"], 0);
                if (tableId <= 0)
                    continue;

                if (!ParseOneTable(tableId, tableObj, newTable)) {
                    if (outError) *outError = "Failed to parse table id: " + std::to_string(tableId);
                    return false;
                }
            }
        }
        else {
            if (outError) *outError = "'tables' must be object or array";
            return false;
        }

        if (!Validate(newTable, outError))
            return false;

        outTable = std::move(newTable);
        return true;
    }

    bool SaveToFile(const std::filesystem::path& filePath, const LootTable& table, std::string* outError)
    {
        // 일단 필요 없다 판단...
        if (outError) *outError = "Not implemented";
        return false;
    }

    bool Validate(const LootTable& table, std::string* outError)
    {
        if (!table.IsValid()) {
            if (outError) *outError = "LootTable validation failed";
            return false;
        }
        return true;
    }
}

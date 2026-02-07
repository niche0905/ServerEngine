#include "pch.h"
#include "JsonConfigLoader.h"

#include <fstream>
#include <sstream>
#include <json/json.h>

/*--------------------
   JsonConfigLoader
--------------------*/

namespace SE::Config
{
    static bool ParseJson(const char* begin, const char* end, ConfigDocument& outDoc, std::string& outError)
    {
        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;
        
        std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
        
        Json::Value root;
        std::string errs;
        
        const bool ok = reader->parse(begin, end, &root, &errs);
        if (not ok) {
            outError = errs;
            outDoc._SetError(errs);
            outDoc._SetValid(false);
            
            return false;
        }
        
        outDoc._MutableRoot() = std::move(root);
        outDoc._SetValid(true);
        
        return true;
    }
    
    bool JsonConfigLoader::LoadFromFile(const std::string& filepath, ConfigDocument& outDoc)
    {
        outDoc.Clear();
        lastError_.clear();
        
        std::ifstream ifs(filepath, std::ios::binary);
        if (not ifs.is_open()) {
            lastError_ = "Failed to open file: " + filepath;
            outDoc._SetError(lastError_);
            return false;
        }
        
        std::ostringstream oss;
        oss << ifs.rdbuf();
        const std::string text = oss.str();
        
        if (text.empty()) {
            lastError_ = "File is empty: " + filepath;
            outDoc._SetError(lastError_);
            return false;
        }
        
        bool ok = ParseJson(text.data(), text.data() + text.size(), outDoc, lastError_);
        if (not ok) 
            return false;
        
        return true;
    }

    bool JsonConfigLoader::LoadFromString(const std::string& text, ConfigDocument& outDoc)
    {
        outDoc.Clear();
        lastError_.clear();
        
        if (text.empty()) {
            lastError_ = "Input text is empty";
            outDoc._SetError(lastError_);
            return false;
        }
        
        bool ok = ParseJson(text.data(), text.data() + text.size(), outDoc, lastError_);
        if (not ok) 
            return false;
        
        return true;
    }
}

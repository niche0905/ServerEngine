#include "pch.h"
#include "FilePathHelper.h"

std::filesystem::path GetExecutableDir()
{
    wchar_t buffer[MAX_PATH];
    DWORD length = ::GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        return {};
    }
    
    return std::filesystem::path{ buffer }.parent_path();
}

std::filesystem::path ResolveConfigPath(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        
        std::string_view arg = argv[i];
        constexpr std::string_view prefix = "--config=";
        if (arg.rfind(prefix, 0) == 0) {
            std::string value(arg.substr(prefix.length()));
            return std::filesystem::path(value);
        }
    }
    
    return GetExecutableDir();
}

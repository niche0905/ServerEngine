#pragma once
#include <windows.h>
#include <filesystem>
#include <string>

std::filesystem::path GetExecutableDir();
std::filesystem::path ResolveConfigPath(int argc, char* argv[]);

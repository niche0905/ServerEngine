#pragma once
#include <string>
#include <filesystem>

struct NetworkConfig
{
    std::string bindIp = "0.0.0.0";
    uint16 gamePort = 8252;
    uint16 loginPort = 8282;
};

struct DataFilesConfig
{
    std::filesystem::path zoneTablePath;
};

struct ServerConfig
{
    NetworkConfig network;
    DataFilesConfig dataFiles;
};

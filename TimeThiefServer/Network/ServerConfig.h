#pragma once
#include <string>

struct NetworkConfig
{
    std::string bindIp = "0.0.0.0";
    uint16 gamePort = 8252;
    uint16 loginPort = 8282;
};

struct ProtocolConfig
{
    int32 version = 1;
};

struct ServerConfig
{
    NetworkConfig network;
    ProtocolConfig protocol;
};

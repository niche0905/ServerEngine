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
    std::filesystem::path lootTablePath;
    std::filesystem::path weaponUpgradeTablePath;
    std::filesystem::path statUpgradeTablePath;
    std::filesystem::path playerSpawnTablePath;
};

struct GameConfig
{
    int32 movementUpdateHz = 10;   // 플레이어 이동 업데이트 주기 (Hz)
    int32 pingIntervalMs = 1000;   // 클라이언트가 Ping 패킷 보내는 간격 (ms)
    
    int32 roomTickIntervalMs = 50;   // Room Tick 간격 (ms) - 20Hz -> 50ms/틱
    
    float zoneDamageTickInterval = 1.0f;   // 존 데미지 Tick 간격 (초)
    
    int32 matchSize = 8;   // 매치당 필요한 플레이어 수
    
    bool testSpawnPoints = true;        // 테스트용 스폰 지점 활성화 (실제 게임을 위한 것이 아닌)
};

struct ServerConfig
{
    NetworkConfig       network;
    DataFilesConfig     dataFiles;
    GameConfig          game;
};

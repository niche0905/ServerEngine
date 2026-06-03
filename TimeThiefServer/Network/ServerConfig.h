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
    std::filesystem::path mapFilePath;
    std::filesystem::path navMeshFilePath;
    std::filesystem::path zoneTablePath;
    std::filesystem::path lootTablePath;
    std::filesystem::path monsterTemplateTablePath;
    std::filesystem::path storeEntryTablePath;
    std::filesystem::path weaponTablePath;
    std::filesystem::path weaponUpgradeTablePath;
    std::filesystem::path statUpgradeTablePath;
    std::filesystem::path playerSpawnTablePath;
    std::filesystem::path npcAiTablePath;
    std::filesystem::path placementInteractionTablePath;
    std::filesystem::path placementMonsterTablePath;
};

struct EconomyConfig
{
    int32 initialTimePoint = 1000;            // 플레이어 초기 재화
    int32 respawnCostTimePoint = 100;        // 리스폰 비용
    int32 playerKillRobberyTimePoint = 100;  // 플레이어 처치 시 강탈량
    int32 zoneDamageTimePointMultiplier = 10; // 존 데미지 1당 차감할 재화
    int32 chestMoneyRewardMin = 60;          // 상자 화폐 보상 최소값
    int32 chestMoneyRewardMax = 120;         // 상자 화폐 보상 최대값
};

struct GameConfig
{
    int32 movementUpdateHz = 10;   // 플레이어 이동 업데이트 주기 (Hz)
    int32 pingIntervalMs = 1000;   // 클라이언트가 Ping 패킷 보내는 간격 (ms)
    
    int32 roomTickIntervalMs = 50;   // Room Tick 간격 (ms) - 20Hz -> 50ms/틱
    
    float zoneDamageTickInterval = 1.0f;   // 존 데미지 Tick 간격 (초)

    EconomyConfig economy;
    
    int32 matchSize = 8;   // 매치당 필요한 플레이어 수
    
    bool enablePartialMatch = false;   // 불완전 매칭 허용 여부
    int32 minPartialMatchSize = 2;   // 불완전 매칭 허용을 위한 최소 플레이어 수 (이 수 이상일 때 불완전 매칭 허용 시)
    int32 partialMatchWaitTimeSec = 10;   // 불완전 매칭 대기 시간 (초)
    
    bool testSpawnPoints = true;        // 테스트용 스폰 지점 활성화 (실제 게임을 위한 것이 아닌)
};

struct ServerConfig
{
    NetworkConfig       network;
    DataFilesConfig     dataFiles;
    GameConfig          game;
};

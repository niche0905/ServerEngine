#pragma once

enum class RespawnState : uint8
{
    None = 0,       // 리스폰 기능 없음
    
    Alive,
    Dead,
    Scheduled,      // 리스폰 예약됨
    Respawning,     // 리스폰 진행 중
};

struct RespawnPolicy
{
    bool enabled{true};
    
    uint32 delayMs{5000};       // 리스폰 대기 시간 (밀리초)
    uint32 invulMs{1500};       // 리스폰 후 무적 시간 (밀리초)
    
    // 리스폰 위치 관련 설정
    bool useSpawnPoint{true};   // 스폰 포인트 사용 여부
    
};

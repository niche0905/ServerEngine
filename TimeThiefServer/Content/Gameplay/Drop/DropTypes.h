#pragma once

enum class DropMode : uint8
{
    CorpseBox,  // 시체 상자 (배틀그라운드나 에이펙스 레전드 차용)
    Scatter,    // 흩뿌리기 (디아블로, 포트나이트 차용)
};

enum class DropReason : uint8
{
    Unknown = 0,        // 알 수 없음
    Death,
    Manual,
    Script,
};

struct DropPolicy
{
    DropMode mode{DropMode::CorpseBox};     // 드롭 모드: 기본은 시체 상자
    
    float moneyDropRate{0.7f};              // 돈 드롭률: 기본 70% <- 보유 총 재화에서 계산
    bool dropConsumables{true};             // 소모품 드롭 여부: 기본 true
    
    int32 maxScatterItemSpawns{12};         // 흩뿌리기 모드에서 최대 아이템 스폰 개수: 기본 12개
    float scatterRadius{120.0f};            // 흩뿌리기 모드에서 아이템 스폰 반경: 기본 120 유닛
};

struct DropSpawnPolicy  // DropSpawnService에 전달되는 스폰 정책(표현 관련)
{
    DropMode mode{DropMode::CorpseBox};
    int32 maxScatterItemSpawns{12};
    float scatterRadius{120.0f};
};

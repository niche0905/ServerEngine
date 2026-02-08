#pragma once

using CooldownId = uint32;

enum class CooldownStartMode : uint8
{
    FromNow = 0,    // 현재 시점부터 쿨다운 시작
    FromEnd,        // 기존 쿨다운 종료 시점부터 연장
};

struct CooldownResult
{
    bool ok{false};
    uint64 nowMs{0};
    uint64 endMs{0};
    uint64 remainingMs{0};
};

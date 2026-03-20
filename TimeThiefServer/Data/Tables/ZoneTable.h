#pragma once

struct ZonePhaseData
{
    float Radius = 0.0f;
    float DamagePerSecond = 0.0f;
    float WaitTimeSeconds = 0.0f;
    float ShrinkTimeSeconds = 0.0f;
};

struct ZoneTable
{
    std::vector<ZonePhaseData> Phases;
};

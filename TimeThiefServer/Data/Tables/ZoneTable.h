#pragma once
#include <vector>
#include <ostream>

struct ZonePhaseData
{
    float radius = 0.0f;
    float damagePerSecond = 0.0f;
    float waitTimeSeconds = 0.0f;
    float shrinkTimeSeconds = 0.0f;
};

struct ZoneTable
{
    std::vector<ZonePhaseData> phases;
    
    const size_t Phase() const { return phases.size(); }
    
    const ZonePhaseData& GetPhaseData(uint32 phase) const
    {
        if (phase >= phases.size())
            return phases.back();   // 마지막 Phase의 데이터를 반환
        
        return phases[phase];
    }
};

inline std::ostream& operator<<(std::ostream& os, const ZonePhaseData& z)
{
    os << "radius: " << z.radius 
        << ", damagePerSecond: " << z.damagePerSecond
        << ", waitTimeSeconds: " << z.waitTimeSeconds 
        << ", shrinkTimeSeconds: " << z.shrinkTimeSeconds;
    
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const ZoneTable& z)
{
    os << "ZoneTable:\n";
    for (size_t i = 0; i < z.phases.size(); i++) {
        os << "Phase " << i + 1 << " => " << z.phases[i] << "\n";
    }
    
    return os;
}

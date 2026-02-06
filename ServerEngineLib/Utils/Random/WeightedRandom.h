#pragma once
#include <vector>
#include <type_traits>

// -------------------------
// Simple RNG (LCG)
// - deterministic
// - fast
// - NOT crypto-secure
// -------------------------

class Random32
{
public:
    explicit Random32(uint32 seed = 0x12345678u) : state_(seed ? seed : 0x12345678u) {}
    
    uint32 NextU32()
    {
        state_ = 1664525u * state_ + 1013904223u;
        return state_;
    }
    
    uint32 NextU32(uint32 maxExclusive)
    {
        if (maxExclusive == 0) return 0;
        return NextU32() % maxExclusive;
    }
    
    int32 NextI32(int32 minInclusive, int32 maxInclusive)
    {
        if (minInclusive > maxInclusive) return minInclusive;
        const uint32 span = static_cast<uint32>(maxInclusive - minInclusive + 1);
        return minInclusive + static_cast<int32>(NextU32(span));
    }
    
    double Next01()
    {
        // 24비트 정밀도의 [0.0, 1.0) 실수 반환
        return static_cast<double>(NextU32() & 0x00FFFFFFu) / static_cast<double>(0x01000000u);
    }
    
    bool Chance(double p)
    {
        if (p <= 0.0) return false;
        if (p >= 1.0) return true;
        return Next01() < p;
    }
    
private:
    uint32 state_;
    
};

template<typename TWeight>
inline bool IsPositiveWeight(TWeight w)
{
    if constexpr (std::is_floating_point_v<TWeight>)
        return w > static_cast<TWeight>(0);
    else
        return w > 0;
}

template<typename WeightGetter>
inline int32 ChooseIndexByWeight(int32 count, WeightGetter&& weightGetter, Random32& rng)
{
    if (count <= 0) return -1;
    
    uint64 total = 0;
    for (int32 i = 0; i < count; ++i) {
        auto w = weightGetter(i);
        if (IsPositiveWeight(w)) {
            total += static_cast<uint64>(w);
        }
    }
    
    if (total == 0) return -1;
    
    uint64 roll = static_cast<uint64>(rng.NextU32()) % total;
    
    uint64 acc = 0;
    for (int32 i = 0; i < count; ++i) {
        auto w = weightGetter(i);
        if (not IsPositiveWeight(w)) continue;
        
        acc += static_cast<uint64>(w);
        if (roll < acc)
            return i;
    }
    
    return -1;
}

template<typename WeightGetter>
inline void ChooseManyIndicesByWeight(int32 count, int32 k, bool allowDuplicates, WeightGetter&& weightGetter, Random32& rng, std::vector<int32>& outIndices)
{
    outIndices.clear();
    if (count <= 0 or k <= 0) return;
    
    if (allowDuplicates)
    {
        outIndices.reserve(static_cast<size_t>(k));
        for (int32 n = 0; n < k; ++n) {
            int32 idx = ChooseIndexByWeight(count, weightGetter, rng);
            if (idx < 0) break;
            outIndices.push_back(idx);
        }
        
        return;
    }
    
    std::vector<int32> candidates;
    candidates.reserve(static_cast<size_t>(count));
    for (int32 i = 0; i < count; ++i) {
        auto w = weightGetter(i);
        if (IsPositiveWeight(w)) {
            candidates.push_back(i);
        }
    }
    
    if (candidates.empty()) return;
    
    const int32 want = (k < static_cast<int32>(candidates.size())) ? k : static_cast<int32>(candidates.size());
    outIndices.reserve(static_cast<size_t>(want));
    
    for (int32 n = 0; n < want; ++n) {
        auto candWeightGetter = [&](int32 candIdx) -> int64
        {
            const int32 originalIndex = candidates[static_cast<size_t>(candIdx)];
            return static_cast<int64>(weightGetter(originalIndex));
        };
        
        const int32 chosenCandPos = ChooseIndexByWeight(static_cast<int32>(candidates.size()), candWeightGetter, rng);
        if (chosenCandPos < 0) break;
        
        const int32 chosenOriginal = candidates[static_cast<size_t>(chosenCandPos)];
        outIndices.push_back(chosenOriginal);
        
        candidates[static_cast<size_t>(chosenOriginal)] = candidates.back();
        candidates.pop_back();
        
        if (candidates.empty()) break;
    }
}

inline int32 RollRangeI32(int32 minInclusive, int32 maxInclusive, Random32& rng)
{
    return  rng.NextI32(minInclusive, maxInclusive);
}
#pragma once
#include "Content/Object/ObjectId.h"

using EffectId = uint32;
using EffectTag = uint32;

enum class EffectPolarity : uint8
{
    Buff = 0,
    Debuff,
    Neutral,
};

enum class EffectDispelType : uint8
{
    None = 0,
    DispelBuff,
    DispelDebuff,
    DispelAll,
};

enum class EffectApplyResult : uint8
{
    Applied = 0,        // 새로 적용됨
    Refreshed,          // 기존 효과 갱신
    Stacked,            // 스택 증가
    Rejected,           // 적용 거부됨 (면역, 중복 불가 등의 이유)
};

struct EffectApplyContext
{
    ObjectId source{};      // 효과를 적용한 오브젝트
    ObjectId instigator{};  // 효과의 원인 (예: 스킬 시전자)
    int32 skillId{0};       // 효과를 적용한 스킬 ID (있다면)
    uint64 nowMs{0};
};

struct EffectInstance
{
    EffectId id{0};
    EffectPolarity polarity{EffectPolarity::Neutral};
    
    int32 stack{1};
    int32 maxStack{1};
    
    uint64 startMs{0};
    uint64 expireMs{0};     // 0이면 영구 효과
    
    EffectTag tags{0};      // 면역/디스펠/기타 태그
    ObjectId source{};      // 효과를 적용한 오브젝트 
    
    bool IsPermanent() const { return expireMs == 0; }
    bool IsExpired(uint64 nowMs) const { return ((expireMs != 0) and (nowMs < startMs)); }
};

struct EffectDef
{
    EffectId id{0};
    EffectPolarity polarity{EffectPolarity::Neutral};
    
    uint32 durationMs{0};   // 0이면 영구 효과
    int32 maxStack{1};
    
    bool unique{true};     // true 면 중복 불가, false 면 중복 가능
    bool refreshDurationOnReapply{true}; // true 면 재적용 시 지속시간 갱신
    
    EffectTag tags{0};
};

struct DispelRequest
{
    EffectDispelType type{EffectDispelType::None};
    EffectTag mustHaveTags{0};          // 0이면 무시
    EffectTag mustNotHaveTags{0};       // 0이면 무시
    int32 maxRemoveCount{0};            // 0 이면 제한 없음
};

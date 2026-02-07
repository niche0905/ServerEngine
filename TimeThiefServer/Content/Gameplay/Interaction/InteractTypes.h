#pragma once

struct ObjectId;

// 상호작용 결과 코드(서버 검증용)
enum class InteractResultCode : uint8
{
    Ok = 0,
    
    NotInteractable,
    TargetDead,
    InteractorInvalid,
    TooFar,
    NotInView,
    Cooldown,
    Locked,
    Busy,
    Forbidden,
    NotEnoughRequirement,
    InvalidParam,
};

struct InteractContext
{
    ObjectId interactor{}; // 상호작용 주체(플레이어 등)
    
    uint64 nowMs{}; // 현재 시간(밀리초)
    // 필요시 확장:
    // ObjectId itemUsed{};
    // int32_t skillId{};
    // float interactorX, interactorY, interactorZ; // 거리검증용
};

struct InteractRequest
{
    // 클라가 보내는 파라미터(예: "Open", "LootAll", "UseSlot=2")
    int32 actionId{0};
};

struct InteractResult
{
    InteractResultCode code{InteractResultCode::NotInteractable};
    bool accepted{false}; // 서버가 "처리 시도"를 했는지(규칙/상태상 불가면 false)
    int32 actionId{0};
};

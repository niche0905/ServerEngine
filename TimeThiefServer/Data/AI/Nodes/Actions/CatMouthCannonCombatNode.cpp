#include "pch.h"
#include "CatMouthCannonCombatNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;

namespace
{
    constexpr float CannonRange = 2500.0f;

    // 애니메이션 길이
    constexpr float AttackAnimDuration = 1.33f;

    // 실제 포탄이 나가는 타이밍
    constexpr float FireTiming = 0.813f;

    constexpr SE::Math::Vector3 AimOffset{0.0f, 0.0f, 90.0f};

    // 조준 오차. 월드 기준 반지름.
    // 값은 기획에 맞게 조절.
    constexpr float AimConeRadiusAtTarget = 60.0f;
    
    void ResetCombatReservation(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
    }
}


BT::PortsList CatMouthCannonCombatNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatMouthCannonCombatNode::onStart()
{
    elapsedTime_ = 0.0f;
    fired_ = false;
    cancelled_ = false;

    selfNpc_ = nullptr;
    targetPawn_ = nullptr;

    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc_) || selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn_) || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead() || targetPawn_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    const ObjectId selfId = selfNpc_->GetId();

    // 클라이언트: 몽타주 시작, 차징 VFX 시작
    room->NotifyCombatEvent(selfId, CombatEventType::CatCannonCastStart);

    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus CatMouthCannonCombatNode::onRunning()
{
    if (selfNpc_ == nullptr || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    const ObjectId selfId = selfNpc_->GetId();

    if (selfNpc_->IsDead() || targetPawn_->IsDead())
    {
        room->NotifyCombatEvent(selfId, CombatEventType::CatCannonCancel);
        cancelled_ = true;
        ResetCombatReservation(*this);
        return BT::NodeStatus::FAILURE;
    }

    const float dt = room->GetDelta();
    elapsedTime_ += dt;
    
    if (!fired_)
    {
        SE::Math::Vector3 lookDir = targetPawn_->GetPosition() - selfNpc_->GetPosition();
        lookDir.z = 0.0f;
        selfNpc_->LookAtDirection(lookDir);
    }

    if (!fired_ && elapsedTime_ >= FireTiming)
    {
        fired_ = true;

        auto& gameSystem = room->GetRoomGameSystem();
        CombatSystem& combatSystem = gameSystem.GetCombatSystem();

        // TODO: 입 위치 대충 가져와서 Muzzle 위치로 보정 해야함 (AimOffset 이용)
        const SE::Math::Vector3 origin =
            selfNpc_->GetPosition();

        const SE::Math::Vector3 rawTargetPos =
            targetPawn_->GetPosition();

        SE::Math::Vector3 rawDir = rawTargetPos - origin;
        if (rawDir.LengthSq() <= 0.0001f)
        {
            room->NotifyCombatEvent(selfId, CombatEventType::CatCannonCancel);
            cancelled_ = true;
            ResetCombatReservation(*this);
            return BT::NodeStatus::FAILURE;
        }

        rawDir = rawDir.Normalized();

        SE::Physics::Ray visibilityRay(origin, rawDir, CannonRange);

        // 발사 직전에 서버가 최종 검증.
        // 엄폐했거나 시야가 끊겼으면 캐스팅 취소.
        if (!combatSystem.CanSeeTarget(visibilityRay))
        {
            room->NotifyCombatEvent(selfId, CombatEventType::CatCannonCancel);
            cancelled_ = true;
            ResetCombatReservation(*this);
            return BT::NodeStatus::FAILURE;
        }

        const SE::Math::Vector3 aimedTargetPos =
            MakeConeErrorTargetPosition(origin, rawTargetPos, AimConeRadiusAtTarget);

        SE::Math::Vector3 fireDir = aimedTargetPos - origin;
        if (fireDir.LengthSq() <= 0.0001f)
        {
            room->NotifyCombatEvent(selfId, CombatEventType::CatCannonCancel);
            cancelled_ = true;
            ResetCombatReservation(*this);
            return BT::NodeStatus::FAILURE;
        }

        fireDir = fireDir.Normalized();

        // 발사 판정
        room->HandleMonsterFire(selfId, CombatEventType::CatCannonFire, origin, fireDir, CannonRange, 50);
    }

    if (elapsedTime_ >= AttackAnimDuration)
    {
        ResetCombatReservation(*this);
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::RUNNING;
}

void CatMouthCannonCombatNode::onHalted()
{
    if (!fired_ && !cancelled_ && selfNpc_ != nullptr)
    {
        if (auto room = selfNpc_->GetRoom())
        {
            room->NotifyCombatEvent(
                selfNpc_->GetId(),
                CombatEventType::CatCannonCancel
            );
        }
    }

    elapsedTime_ = 0.0f;
    fired_ = false;
    cancelled_ = false;
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
}

SE::Math::Vector3 CatMouthCannonCombatNode::MakeConeErrorTargetPosition(const SE::Math::Vector3& origin,
    const SE::Math::Vector3& targetPos, float radiusAtTarget) const
{
    static thread_local std::mt19937 rng(std::random_device{}());

    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * Pi);

    const float r = radiusAtTarget * std::sqrt(dist01(rng));
    const float theta = distAngle(rng);

    SE::Math::Vector3 forward = targetPos - origin;
    if (forward.LengthSq() <= 0.0001f) {
        return targetPos;
    }

    forward = forward.Normalized();

    SE::Math::Vector3 up{0.0f, 0.0f, 1.0f};

    if (std::abs(forward.Dot(up)) > 0.95f) {
        up = SE::Math::Vector3{1.0f, 0.0f, 0.0f};
    }

    SE::Math::Vector3 right = up.Cross(forward).Normalized();
    SE::Math::Vector3 realUp = forward.Cross(right).Normalized();

    const SE::Math::Vector3 offset =
        right * std::cos(theta) * r +
        realUp * std::sin(theta) * r;

    return targetPos + offset;
}

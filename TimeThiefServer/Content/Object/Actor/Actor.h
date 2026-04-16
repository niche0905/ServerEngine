#pragma once
#include "Content/Object/BaseObject.h"
#include "Content/Object/ObjectId.h"
#include "Content/Gameplay/Collider/ColliderComponent.h"
#include <vector>
#include <memory>

struct SE::Math::Vector3;
class ObjectManager;
class ColliderComponent;

/*---------
   Actor
---------*/
//
// Actor는 게임 내에서 활동하는 모든 개체를 나타내는 기본 클래스입니다.
// Transform 개념을 가지며, Replicaiton 대상입니다
// Component를 가질 수 있습니다
// Spawn/Despawn 라이프사이클 hook을 제공합니다
//

class Actor : public BaseObject
{
public:
   using Vector3 = SE::Math::Vector3;
   
public:
   Actor() = default;
   virtual ~Actor() = default;
   
   Actor(const Actor&) = delete;
   Actor& operator=(const Actor&) = delete;

public:
   virtual EntityType GetEntityType() const { return EntityType::None; }
   
   bool IsPlayer() const { return GetEntityType() == EntityType::Player; }
   bool IsNPC() const { return GetEntityType() == EntityType::NPC; }
   
// Transform 관련 함수들
public:
   const Vector3& GetPosition() const { return position_; }
   void SetPosition(const Vector3& position);
   
   float GetYaw() const { return yaw_; }
   void SetYaw(float yaw);
   
   void SetTransform(const Vector3& position, float yaw);
   
// Collider 관련 함수들
public:
    virtual void ForEachCollider(const std::function<void(ColliderComponent*)>& fn) const override;
   
   const std::vector<std::unique_ptr<ColliderComponent>>& GetColliders() const { return colliders_; }
   
   void SyncColliders();
   
// Replication 관련 함수들
public:
   virtual bool IsReplicated() const { return true; } // 기본적으로 모든 Actor는 Replicated 대상입니다. 예외 시 재정의
   
protected:
   Vector3  position_{};
   float    yaw_{0.0f};
   
   std::vector<std::unique_ptr<ColliderComponent>> colliders_;
   
   // TODO: 추가하기 (회전, 스케일 등 Transform 관련 정보 들)
    
};

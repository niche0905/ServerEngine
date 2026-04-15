#pragma once
#include "ObjectId.h"
#include "ObjectEnum.h"
#include "Content/Gameplay/Collider/ColliderComponent.h"
#include "Content/Gameplay/Replication/ReplicatedState.h"

class Room;
struct ObjectId;

/*--------------
   BaseObject
--------------*/
//
// BaseObject는 모든 게임 오브젝트의 기본 클래스입니다.
//

class BaseObject
{
public:
    BaseObject() = default;
    virtual ~BaseObject() = default;
    
    BaseObject(const BaseObject&) = delete;
    BaseObject& operator=(const BaseObject&) = delete;
    
    ObjectId GetId() const { return id_; }
    RoomId GetRoomId() const { return roomId_; }
    
    std::shared_ptr<Room> GetRoom() const { return room_.lock(); }
    
    ObjectState GetState() const { return state_; }
    bool IsActive() const { return state_ == ObjectState::Alive; }
    bool IsPendingDestroy() const { return state_ == ObjectState::PendingDestroy; }
    
    ObjectFlags GetFlags() const { return flags_; }
    bool IsTickable() const { return HasFlag(flags_, ObjectFlags::Tickable); }
    
    ReplicatedState& GetReplicatedState() { return replicated_; }
    const ReplicatedState& GetReplicatedState() const { return replicated_; }
    
    void MarkReplicationDirty(ReplicationDirty dirtyFlag)
    {
        replicated_.MarkDirty(dirtyFlag);
        
        if (auto room = GetRoom()) {
            // TODO: RoomGameSystem에 ReplicationSystem 추가하기
            // room->GetRoomGameSystem().GetReplicationSystem().MarkDirty(GetId());
        }
    }
    
public:
    virtual void ForEachCollider(const std::function<void(ColliderComponent*)>& fn) const {}
    
// Lifecycle (Room에서만 호출)
public:
    void __Spawn(ObjectId id, RoomId roomId, const std::shared_ptr<Room>& room, ObjectFlags flags)
    {
        id_ = id;
        roomId_ = roomId;
        room_ = room;
        flags_ = flags;
        state_ = ObjectState::Alive;
        
        OnSpawn();
    }
    
    void __RequestDestroy()
    {
        if (state_ != ObjectState::Alive)
            return;
        state_ = ObjectState::PendingDestroy;
        OnPreDestroy();
    }
    
    void __DestroyFinalize()
    {
        if (state_ == ObjectState::Destroyed)
            return;
        OnDestroy();
        state_ = ObjectState::Destroyed;
        room_.reset();
        roomId_ = 0;
    }
    
    void __Tick(float dt)
    {
        if (state_ != ObjectState::Alive)
            return;
        Tick(dt);
    }
    
protected:
    virtual void OnSpawn() {}
    virtual void OnPreDestroy() {}
    virtual void OnDestroy() {}
    virtual void Tick(float dt) {}
    
private:
    ObjectId                id_{};
    RoomId                  roomId_{ 0 };
    std::weak_ptr<Room>     room_;
    ObjectFlags             flags_{ ObjectFlags::None };
    ObjectState             state_{ ObjectState::Destroyed };
    ReplicatedState         replicated_{};
    
};

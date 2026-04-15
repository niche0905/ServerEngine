#pragma once
#include <cassert>
#include "ObjectEnum.h"
#include "BaseObject.h"

class BaseObject;

/*-----------------
   ObjectManager
-----------------*/
//
// ObjectManager는 오브젝트의 생성, 조회, 파괴를 관리합니다.
// 한 Room 혹은 World에서 가진채로 사용됩니다.
//

class ObjectManager
{
public:
   explicit ObjectManager(RoomId roomId)
      : roomId_(roomId)
   {
   }
   
   ~ObjectManager()
   {
      ClearAll();
   }
   
   ObjectManager(const ObjectManager&) = delete;
   ObjectManager& operator=(const ObjectManager&) = delete;
   
   RoomId roomId() const { return roomId_; }
   void SetRoom(const std::shared_ptr<Room>& room) { room_ = room; }
   std::shared_ptr<Room> GetRoom() const { return room_.lock(); }
   
   template<typename T, typename... Args>
   T* Create(ObjectFlags flags, Args&&... args)
   {
      static_assert(std::is_base_of_v<BaseObject, T>, "T must be derived from BaseObject");
      
      const uint16 index = AcquireSlotIndex();
      Entry& e = entries_[index];
      
      const uint16 gen = e.generation;
      const ObjectId id = ObjectId::Make(index, gen);
      
      T* obj = new T(std::forward<Args>(args)...);
      e.ptr = obj;
      e.destroyQueued = false;
      
      auto room = room_.lock();
      obj->__Spawn(id, roomId_, room, flags);
      
      ++aliveCount_;
      return obj;
   }
   
   BaseObject* Find(ObjectId id) const
   {
      if (!id) return nullptr;
      
      const uint16 idx = id.Index();
      if (idx >= entries_.size())
         return nullptr;
      
      const Entry& e = entries_[idx];
      if (e.ptr == nullptr) return nullptr;
      if (e.generation != id.Gen()) return nullptr;
      
      return e.ptr;
   }
   
   template<typename T>
   T* FindAs(ObjectId id) const
   {
      static_assert(std::is_base_of_v<BaseObject, T>, "T must be derived from BaseObject");
      return dynamic_cast<T*>(Find(id));
   }
   
   bool IsValid(ObjectId id) const
   {
      return Find(id) != nullptr;
   }
   
   bool RequestDestroy(ObjectId id)
   {
      BaseObject* obj = Find(id);
      if (!obj) return false;
      if (not obj->IsAlive()) return false;
      
      const uint16 idx = id.Index();
      Entry& e = entries_[idx];
      
      if (!e.destroyQueued) {
         e.destroyQueued = true;
         destroyQueue_.push_back(id);
      }
      
      obj->__RequestDestroy();
      return true;
   }
   
   void SweepDestroy(std::size_t budget = SIZE_MAX)
   {
      std::size_t processed = 0;
      while (not destroyQueue_.empty() and processed < budget ) {
         
         const ObjectId id = destroyQueue_.front();
         destroyQueue_.pop_front();
         ++processed;
         
         const uint16 idx = id.Index();
         if (idx >= entries_.size()) continue;
         
         Entry& e = entries_[idx];
         BaseObject* obj = e.ptr;
         
         if (!obj) continue;
         if (e.generation != id.Gen()) continue;
         
         e.destroyQueued = false;
         
         obj->__DestroyFinalize();
         
         delete obj;
         
         e.ptr = nullptr;
         e.generation = static_cast<uint16>(e.generation + 1);
         freeList_.push_back(idx);
         
         assert(aliveCount_ > 0);;
         --aliveCount_;
      }
   }
   
   template<typename Fn>
   void ForEachAlive(Fn&& fn) const // TEMP: const는 나중에 제거할 수도 있음...
   {
      for (const Entry& e : entries_) {
         if (!e.ptr) continue;
         if (!e.ptr->IsAlive()) continue;
         
         fn(e.ptr);
      }
   }
   
   template<typename Fn>
   void ForEachTickableAlive(Fn&& fn)
   {
      for (const Entry& e : entries_) {
         if (!e.ptr) continue;
         if (!e.ptr->IsAlive()) continue;
         if (!e.ptr->IsTickable()) continue;
         
         fn(e.ptr);
      }
   }
   
   std::size_t AliveCount() const { return aliveCount_; }
   std::size_t Capacity() const { return entries_.size(); }
   
   void Reserve(std::size_t n)
   {
      if (n <= entries_.size()) return;
      
      const std::size_t old = entries_.size();
      entries_.resize(n);
      
      for (uint16 i = static_cast<uint16>(old); i < n; ++i) {
         entries_[i].generation = 1;
         entries_[i].ptr = nullptr;
         entries_[i].destroyQueued = false;
         freeList_.push_back(i);
      }
   }
   
   void ClearAll()
   {
      for (uint16 i = 0; i < entries_.size(); ++i) {
         if (entries_[i].ptr) {
            ObjectId id = ObjectId::Make(i, entries_[i].generation);
            RequestDestroy(id);
         }
      }
      
      SweepDestroy(SIZE_MAX);
      
      destroyQueue_.clear();
   }
   
private:
   struct Entry
   {
      BaseObject* ptr = nullptr;
      uint16 generation = 1;
      bool destroyQueued = false;
   };
   
   uint16 AcquireSlotIndex()
   {
      if (freeList_.empty()) {
         const std::size_t old = entries_.size();
         const std::size_t next = (old == 0) ? 256 : old * 2;
         Reserve(next);
      }
      
      const uint16 idx = freeList_.back();
      freeList_.pop_back();
      
      assert(idx < entries_.size());
      assert(entries_[idx].ptr == nullptr);
      
      return idx;
   }
   
   
private:
   RoomId roomId_;
   std::weak_ptr<Room> room_;
   std::vector<Entry> entries_;
   std::vector<uint16> freeList_;
   std::deque<ObjectId> destroyQueue_;
   std::size_t aliveCount_{ 0 };
   
};
#include "pch.h"
#include "ReplicationSystem.h"

/*---------------------
   ReplicationSystem
---------------------*/

void ReplicationSystem::RegisterObject(RepObjectId obj)
{
   if (obj == RepObjectId{}) return;
   
   auto& st = objects_[obj];
   st.object = obj;
}

void ReplicationSystem::UnregisterObject(RepObjectId obj)
{
   if (obj == RepObjectId{}) return;
   objects_.erase(obj);
   
   // TODO: interest에 남아있는 것들 정리 할 필요가 있다
}

void ReplicationSystem::MarkDirty(RepObjectId obj, RepField f)
{
   auto it = objects_.find(obj);
   if (it == objects_.end()) {
      // 등록이 안된 오브젝트라면 일단 등록
      RegisterObject(obj);
      it = objects_.find(obj);
      if (it == objects_.end()) return;
   }
   
   it->second.MarkDirty(f);
}

void ReplicationSystem::PushEvent(const RepEvent& event)
{
   events_.Push(event);
}

void ReplicationSystem::FlushSpawnDespawn(const InterestResult& interest)
{
   for (const auto& [cid, curSet] : interest.visible) {
      
      InterestSet& prevSet = lastVisible_[cid];
      
      for (const auto& obj : curSet) {
         if (prevSet.find(obj) == prevSet.end()) {
            sink_.SendSpawn(cid, obj);
         }
      }
      
      for (auto it = prevSet.begin(); it != prevSet.end(); ) {
         const RepObjectId obj = *it;
         if (curSet.find(obj) == curSet.end()) {
            sink_.SendDespawn(cid, obj);
            it = prevSet.erase(it);
         } else {
            ++it;
         }
      }
      
      prevSet = curSet;
   }
}

void ReplicationSystem::FlushState(const InterestResult& interest)
{
   for (const auto& [cid, vis] : interest.visible) {
      
      for (const auto& obj : vis) {
         auto it = objects_.find(obj);
         if (it == objects_.end()) continue;
         
         ReplicatedState& state = it->second;
         if (not state.IsDirty()) continue;
         
         sink_.SendState(cid, obj, state.dirtyMask);
      }
   }
   
   for (auto& [obj, st] : objects_) {
      if (st.IsDirty()) {
         st.lastFlushSeq = frame_.seq;
         st.ClearDirty();
      }
   }
}

void ReplicationSystem::FlushEvents(const InterestResult& interest)
{
   if (events_.Empty()) return;
   
   for (const auto& [cid, _] : interest.visible) {
      for (const auto& event : events_.Events()) {
         sink_.SendEvent(cid, event);
      }
   }
}

#pragma once
#include "Content/Shared/BaseComponent.h"
#include "CooldownTypes.h"
#include <unordered_map>
#include <algorithm>

/*---------------------
   CooldownComponent
---------------------*/
//
// CooldownComponent는 쿨다운 상태를 관리하는 컴포넌트입니다.
//

class CooldownComponent : public BaseComponent
{
public:
   void Init(ObjectId owner)
   {
      SetOwner(owner);
      
      enabled_ = true;
      cdEndMs_.clear();
   }
   
   void SetEnabled(bool enable) { enabled_ = enable; }
   bool IsEnabled() const { return enabled_; }
   
   bool IsReady(CooldownId id, uint64 nowMs) const
   {
      if (not enabled_) return true;      // 비활성화된 경우 항상 준비된 상태
      auto it = cdEndMs_.find(id);
      if (it == cdEndMs_.end()) return true; // 쿨다운이 설정되지 않은 경우 준비된 상태
      return nowMs >= it->second;        // 현재 시각이 종료 시각 이후인지 확인
   }
   
   uint64 GetRemainingMs(CooldownId id, uint64 nowMs) const
   {
      auto it = cdEndMs_.find(id);
      if (it == cdEndMs_.end()) return 0; // 쿨다운이 설정되지 않은 경우 남은 시간이 없음
      return (nowMs >= it->second) ? 0 : (it->second - nowMs);
   }
   
   CooldownResult Start(CooldownId id, uint64 nowMs, uint32 durationMs, CooldownStartMode mode = CooldownStartMode::FromNow)
   {
      CooldownResult result{};
      result.nowMs = nowMs;
      
      if (not enabled_) {
         result.ok = true;
         result.endMs = nowMs;
         result.remainingMs = 0;
         return result; // 비활성화된 경우 즉시 성공
      }
      
      if (id == 0 or durationMs == 0) {
         result.ok = true;
         result.endMs = nowMs;
         result.remainingMs = 0;
         return result; // ID가 0이거나 지속 시간이 0인 경우 즉시 성공
      }
      
      uint64 newEnd = nowMs + static_cast<uint64>(durationMs);
      
      auto it = cdEndMs_.find(id);
      if (it != cdEndMs_.end()) {
         const uint64 curEng = it->second;
         
         if (mode == CooldownStartMode::FromEnd and curEng > nowMs)
            newEnd = curEng + static_cast<uint64>(durationMs);
         
         it->second = newEnd;
      }
      else {
         cdEndMs_.emplace(id, newEnd);
      }
      
      result.ok = true;
      result.endMs = newEnd;
      result.remainingMs = (newEnd > nowMs) ? (newEnd - nowMs) : 0;
      
      return result;
   }
   
   CooldownResult TryConsume(CooldownId id, uint64 nowMs, uint32 durationMs, CooldownStartMode mode = CooldownStartMode::FromNow)
   {
      if (IsReady(id, nowMs)) {
         return Start(id, nowMs, durationMs, mode);
      }
      
      CooldownResult result{};
      result.nowMs = nowMs;
      
      result.ok = false;
      const uint64 endMs = cdEndMs_.find(id)->second;
      result.endMs = endMs;
      result.remainingMs = (endMs > nowMs) ? (endMs - nowMs) : 0;
      
      return result;
   }
   
   bool Clear(CooldownId id)
   {
      return cdEndMs_.erase(id) > 0;
   }
   
   void ClearAll()
   {
      cdEndMs_.clear();
   }
   
   uint64 GetNextExpireMs(uint64 nowMs) const
   {
      uint64 next = 0;
      for (const auto& [id, end] : cdEndMs_) {
         (void)id;
         if (end <= nowMs) continue;
         if (next == 0 or end < next) {
            next = end;
         }
      }
      
      return next;
   }
   
   // 만료된 엔트리 청소: 가끔씩 호출
   int32 CleanupExpired(uint64 nowMs)
   {
      int32 removed = 0;
      
      for (auto it = cdEndMs_.begin(); it != cdEndMs_.end(); ) {
         
         if (nowMs >= it->second) {
            it = cdEndMs_.erase(it);
            ++removed;
         }
         else {
            ++it;
         }
      }
      
      return removed;
   }
   
private:
   bool enabled_{true};
   std::unordered_map<CooldownId, uint64> cdEndMs_; // 쿨다운 ID별 종료 시각 (밀리초)
   
};
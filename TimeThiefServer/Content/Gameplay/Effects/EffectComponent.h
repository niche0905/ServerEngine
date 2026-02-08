#pragma once
#include "Content/Shared/BaseComponent.h"
#include "Content/Object/BaseObject.h"
#include "IEffect.h"
#include <vector>
#include <algorithm>

class ObjectManager;

/*-------------------
   EffectComponent
-------------------*/
//
// EffectComponent는 객체에 적용된 효과(버프/디버프 등)를 관리하는 컴포넌트입니다.
//

class EffectComponent : public BaseComponent
{
public:
   void Init(ObjectId owner)
   {
      SetOwner(owner);
      
      effects_.clear();
      enabled_ = true;
   }
   
   void SetEnabled(bool value) { enabled_ = value; }
   bool IsEnabled() const { return enabled_; }
   
   const std::vector<EffectInstance>& GetEffects() const { return effects_; }
   
   bool HasEffect(EffectId id) const
   {
      return FindIndexById(id) >= 0;
   }
   
   EffectApplyResult ApplyEffect(ObjectManager& om, ObjectId target, IEffect& effect, const EffectApplyContext& ctx)
   {
      if (not enabled_) return EffectApplyResult::Rejected;
      
      const EffectDef& def = effect.GetDef();
      if (def.id == 0) return EffectApplyResult::Rejected;
      
      const int32 idx = FindIndexById(def.id);
      
      if (idx < 0) {    // 없었다면
         EffectInstance inst{};
         inst.id = def.id;
         inst.polarity = def.polarity;
         inst.stack = 1;
         inst.maxStack = def.maxStack;
         inst.startMs = ctx.nowMs;
         inst.expireMs = (def.durationMs > 0) ? (ctx.nowMs + def.durationMs) : 0;
         inst.tags = def.tags;
         inst.source = ctx.source;
         
         effects_.push_back(inst);
         
         effect.OnApply(om, target, ctx);
         return EffectApplyResult::Applied;
      }
      
      // 이미 있었다면
      EffectInstance& inst = effects_[static_cast<size_t>(idx)];
      
      bool didStack = false;
      if (inst.stack < inst.maxStack) {
         inst.stack += 1;
         didStack = true;
      }
      
      bool didRefresh = false;
      if (def.refreshDurationOnReapply) {
         if (def.durationMs > 0) {
            inst.expireMs = std::max(inst.expireMs, ctx.nowMs + def.durationMs);
         }
         
         didRefresh = true;
      }
      
      if (didStack) {
         effect.OnRefresh(om, target, ctx);
         return EffectApplyResult::Stacked;
      }
      
      if (didRefresh) {
         effect.OnRefresh(om, target, ctx);
         return EffectApplyResult::Refreshed;
      }

      return EffectApplyResult::Rejected;
   }
   
   bool RemoveEffect(ObjectManager& om, ObjectId target, EffectId id, uint64 nowMs, IEffect* hook = nullptr)
   {
      const int32 idx = FindIndexById(id);
      if (idx < 0) return false;
      
      if (hook)
         hook->OnRemove(om, target, nowMs);
      
      SwapPop(static_cast<size_t>(idx));
      return true;
   }
   
   int32 Dispel(ObjectManager& om, ObjectId target, const DispelRequest& req, uint64 nowMs)
   {
      if (req.type == EffectDispelType::None) return 0;
      
      int32 removed = 0;
      
      for (int32 i = static_cast<int32>(effects_.size()) - 1; i >= 0; --i) {
         if (req.maxRemoveCount > 0 and removed >= req.maxRemoveCount) break;    // 제거 개수 만족
         
         const EffectInstance& inst = effects_[static_cast<size_t>(i)];
         
         if (req.type == EffectDispelType::DispelBuff and inst.polarity != EffectPolarity::Buff) continue;
         if (req.type == EffectDispelType::DispelDebuff and inst.polarity != EffectPolarity::Debuff) continue;
         
         if (req.mustHaveTags != 0 and (inst.tags & req.mustHaveTags) != req.mustHaveTags) continue;
         if (req.mustNotHaveTags != 0 and (inst.tags & req.mustNotHaveTags) != 0) continue;
         
         // 제거
         SwapPop(static_cast<size_t>(i));
         ++removed;
      }
      
      (void)om; (void)target; (void)nowMs;
      return removed;
   }
   
   // 만료된 효과 제거, 반환값은 제거된 효과 개수
   int32 UpdateExpired(ObjectManager& om, ObjectId target, uint64 nowMs)
   {
      int32 removed = 0;
      
      for (int32 i = static_cast<int32>(effects_.size()) - 1; i >= 0; --i) {
         const auto& inst = effects_[static_cast<size_t>(i)];
         if (not inst.IsExpired(nowMs)) continue;
         
         SwapPop(static_cast<size_t>(i));
         ++removed;
      }
      
      (void)om; (void)target;
      return removed;
   }
   
   uint64 GetNextExpired() const
   {
      uint64 next = 0;
      for (const auto& inst : effects_) {
         if (inst.expireMs == 0) continue;
         if (next == 0 or inst.expireMs < next) {
            next = inst.expireMs;
         }
      }
      
      return next;
   }
   
private:
   int32 FindIndexById(EffectId id) const
   {
      for (size_t i = 0; i < effects_.size(); ++i) {
         if (effects_[i].id == id) {
            return static_cast<int32>(i);
         }
      }
      
      return -1;
   }
   
   void SwapPop(size_t index)
   {
      if (index >= effects_.size()) return;
      effects_[index] = effects_.back();
      effects_.pop_back();
   }
   
private:
   bool enabled_{true};
   std::vector<EffectInstance> effects_;
   
};
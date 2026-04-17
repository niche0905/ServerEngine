#pragma once
#include "Content/Shared/BaseComponent.h"
#include "Content/Object/ObjectId.h"
#include "WalletTypes.h"
#include <unordered_map>

class ObjectManager;

/*-------------------
   WalletComponent
-------------------*/
//
// WalletComponent는 오브젝트의 지갑 정보를 관리합니다.
//

class WalletComponent : public BaseComponent
{
public:
   void Init(ObjectId owner)
   {
      SetOwner(owner);
      
      SetBalanceUnsafe(CurrencyType::TimePoint, 1000);     // TEMP: 초기 재화 1000
                                                                  // TODO: 이 값 Config로 빼기
   }
   
   CurrencyAmount GetBalance(CurrencyType currency) const
   {
      auto it = balances_.find(currency);
      return (it != balances_.end()) ? it->second : 0;
   }
   
   bool CanSpend(CurrencyType currency, CurrencyAmount amount) const
   {
      if (amount <= 0) return true;
      return GetBalance(currency) >= amount;
   }
   
   MoneyChangeResult AddMoney(ObjectManager& om, CurrencyType currency, CurrencyAmount amount, const MoneyChangeContext& ctx)
   {
      MoneyChangeResult result{};
      result.currency = currency;
      
      if (amount <= 0) return result;
      
      const CurrencyAmount before = GetBalance(currency);
      const CurrencyAmount after = before + amount;
      
      balances_[currency] = after;
      
      result.before = before;
      result.after = after;
      result.delta = amount;
      
      result.accepted = true;
      
      return result;
   }
   
   MoneyChangeResult SpendMoney(ObjectManager& om, CurrencyType currency, CurrencyAmount amount, const MoneyChangeContext& ctx)
   {
      MoneyChangeResult result{};
      result.currency = currency;
      
      if (amount <= 0) return result;
      
      const CurrencyAmount before = GetBalance(currency);
      if (before < amount) {
         return result; // 잔액 부족
      }
      
      const CurrencyAmount after = before - amount;
      
      balances_[currency] = after;
      
      result.before = before;
      result.after = after;
      result.delta = -amount;
      
      result.accepted = true;
      
      return result;
   }
   
   // --- utility ---
   void SetBalanceUnsafe(CurrencyType currency, CurrencyAmount amount)
   {
      balances_[currency] = amount;
   }
   
   void Clear()
   {
      balances_.clear();
   }
   
private:
   std::unordered_map<CurrencyType, CurrencyAmount> balances_;
   
};

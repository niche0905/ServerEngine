#pragma once
#include "WalletTypes.h"

class ObjectManager;

/*----------------
   IWalletOwner
----------------*/
//
// IWalletOwner는 지갑 소유자가 구현해야 하는 인터페이스입니다.
//

class IWalletOwner
{
public:
    virtual ~IWalletOwner() = default;

    virtual CurrencyAmount GetBalance(CurrencyType currency) const = 0;
    virtual bool CanSpend(CurrencyType currency, CurrencyAmount amount) const = 0;

    virtual MoneyChangeResult AddMoney(CurrencyType currency,
                                       CurrencyAmount amount,
                                       const MoneyChangeContext& ctx) = 0;

    virtual MoneyChangeResult SpendMoney(CurrencyType currency,
                                         CurrencyAmount amount,
                                         const MoneyChangeContext& ctx) = 0;
};

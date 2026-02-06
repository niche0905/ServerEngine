#pragma once
#include "Content/Enum/WalletTypes.h"

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

    virtual int64 GetBalance(CurrencyId currency) const = 0;
    virtual bool CanSpend(CurrencyId currency, int64 amount) const = 0;

    virtual MoneyChangeResult AddMoney(ObjectManager& om,
                                       CurrencyId currency,
                                       int64 amount,
                                       const MoneyChangeContext& ctx) = 0;

    virtual MoneyChangeResult SpendMoney(ObjectManager& om,
                                         CurrencyId currency,
                                         int64 amount,
                                         const MoneyChangeContext& ctx) = 0;
};

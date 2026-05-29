#pragma once
#include "ServiceBase.h"
#ifdef USE_RIO
#include "RIO/RioServerService.h"
#else
#include "IOCP/IocpServerService.h"
#endif

class ServiceBase;

class NetworkServiceFactory
{
public:
    static std::shared_ptr<ServiceBase> CreateDefaultNetworkService(
        const NetAddr& addr,
        SessionFactory sessionFactory,
        int32 maxSessionCount)
    {
#ifdef USE_RIO
        return std::make_shared<RioServerService>(addr, sessionFactory, maxSessionCount);
#else
        return std::make_shared<IocpServerService>(addr, sessionFactory, maxSessionCount);
#endif
    }
};

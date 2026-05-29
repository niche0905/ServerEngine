#pragma once
#include "ServiceBase.h"
#include "IOCP/IocpServerService.h"
#include "RIO/RioServerService.h"

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
        return CreateNetworkService(
            BackendType::RIO,
            addr,
            std::move(sessionFactory),
            maxSessionCount);
#else
        return CreateNetworkService(
            BackendType::IOCP,
            addr,
            std::move(sessionFactory),
            maxSessionCount);
#endif
    }
    
    static std::shared_ptr<ServiceBase> CreateNetworkService(
        BackendType backend, NetAddr addr, SessionFactory factory, int32 maxSessionCount)
    {
        switch (backend)
        {
        case BackendType::IOCP:
            return std::make_shared<IocpServerService>(addr, factory, maxSessionCount);
            
        case BackendType::RIO:
            return std::make_shared<RioServerService>(addr, factory, maxSessionCount);
            
        default:
            return nullptr;
        }
    }
    
};

#pragma once
#include "ServiceBase.h"
#include "IOCP/IocpServerService.h"

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
            // RIO 기반 네트워크 서비스 구현이 필요할 때 여기에 추가
            return nullptr;
            
        default:
            return nullptr;
        }
    }
    
};

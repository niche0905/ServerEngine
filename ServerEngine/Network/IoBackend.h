#pragma once

enum class BackendType { IOCP, RIO };

enum class ServiceType
{
    IocpService,
	IocpServerService,
};


#ifdef USE_RIO
///////////////
/// RIO USE ///
///////////////

#include "RioCore.h"

//using SelectedService = ServiceType::RioService;
using IoCoreType = class RioCore;

#else // USE_IOCP
////////////////
/// IOCP USE ///
////////////////

#include "IocpCore.h"
#include "IocpEvent.h"
#include "IocpConnectionEvent.h"
#include "IocpIoEvent.h"

//using SelectedService = ServiceType::IocpService;
using IoCoreType = IocpCore;

using ConnectEvent = IocpConnectEvent;
using DisconnectEvent = IocpDisconnectEvent;
using AcceptEvent = IocpAcceptEvent;
using RecvEvent = IocpRecvEvent;
using SendEvent = IocpSendEvent;

#endif

class IoBackend
{
public:
    // TODO: 좀 더 고민해보기
    //static std::shared_ptr<Service> MakeService(ServiceType type, NetAddress addr, SessionFactory factory, int maxSessionCount)
    //{
    //    switch (type)
    //    {
    //    case ServiceType::IOCP:
    //        //return make_shared<IocpService>(addr, make_shared<IocpCore>(), factory, maxSessionCount);
    //    case ServiceType::RIO:
    //        //return make_shared<RioService>(addr, make_shared<RioCore>(), factory, maxSessionCount);
    //    }
    //    return nullptr;
    //}

private:
	// TODO: Seperate IoService classes for IOCP and RIO
    //static shared_ptr<Service> MakeIocpService(NetAddress addr, SessionFactory sf, int max)
    //{
    //    auto core = make_shared<IocpCore>();
    //    return make_shared<IocpService>(addr, core, sf, max);
    //}

    //static shared_ptr<Service> MakeRioService(NetAddress addr, SessionFactory sf, int max)
    //{
    //    auto core = make_shared<RioCore>();
    //    return make_shared<RioService>(addr, core, sf, max);
    //}

};


#pragma once

enum class BackendType { IOCP, RIO };

enum class ServiceType
{
    IocpService,
	IocpServerService,
    
    RioService,
    RioServerService
};

// #define USE_RIO

#ifdef USE_RIO
///////////////
/// RIO USE ///
///////////////

#include "Core/IoCore/RioCore/RioCore.h"
#include "Network/Event/RIO/RioEvent.h"
#include "Network/Event/RIO/RioIoEvent.h"

constexpr uint32 RioSendBlockSize = 4096;
constexpr uint32 RioSendBlockCount = 8192;

//using SelectedService = ServiceType::RioService;
using IoCoreType = class RioCore;
using NetworkSession = class RioSession;
using NetworkSendBuffer = RioSendBuffer;

using RecvEvent = RioRecvEvent;
using SendEvent = RioSendEvent;

#else // USE_IOCP
////////////////
/// IOCP USE ///
////////////////

#include "Core/IoCore/IocpCore/IocpCore.h"
#include "Network/Event/IOCP/IocpEvent.h"
#include "Network/Event/IOCP/IocpConnectionEvent.h"
#include "Network/Event/IOCP/IocpIoEvent.h"

//using SelectedService = ServiceType::IocpService;
using IoCoreType = class IocpCore;
using NetworkSession = class IocpSession;
using NetworkSendBuffer = SendBuffer;

using ConnectEvent = IocpConnectEvent;
using DisconnectEvent = IocpDisconnectEvent;
using AcceptEvent = IocpAcceptEvent;
using RecvEvent = IocpRecvEvent;
using SendEvent = IocpSendEvent;

#endif

using SendBufferRef = std::shared_ptr<NetworkSendBuffer>;

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


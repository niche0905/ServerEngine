#pragma once

#ifdef USE_RIO
///////////////
/// RIO USE ///
///////////////

#include "RioCore.h"

using IoCoreType = class RioCore;

#else // USE_IOCP
////////////////
/// IOCP USE ///
////////////////

#include "IocpCore.h"
#include "IocpEvent.h"
#include "IocpConnectionEvent.h"
#include "IocpIoEvent.h"

using IoCoreType = IocpCore;

using ConnectEvent = IocpConnectEvent;
using DisconnectEvent = IocpDisconnectEvent;
using AcceptEvent = IocpAcceptEvent;
using RecvEvent = IocpRecvEvent;
using SendEvent = IocpSendEvent;

#endif

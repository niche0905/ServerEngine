#include "pch.h"
#include "RioServerService.h"
#include "Network/SocketUtils.h"
#include "Utils/Logger/ConsoleLogger.h"

/*--------------------
   RioServerService
--------------------*/

RioServerService::RioServerService(NetAddr address, SessionFactory factory, int32 maxSessionCount)
    : RioService(ServiceType::RioServerService, address, factory, maxSessionCount)
{
}

RioServerService::~RioServerService()
{
    StopService();
}

bool RioServerService::Start()
{
    if (false == RioService::Start()) {
        return false;
    }

    if (StartListening() == false) {
        return false;
    }

    acceptRunning_.store(true);
    acceptThread_ = std::thread([this]()
    {
        AcceptLoop();
    });
    
    consoleLogger->Log(Color::Green, L"[Service] RioServerService started.\n");

    return true;
}

void RioServerService::StopService()
{
    acceptRunning_.store(false);
    SocketUtils::Close(listenSocket_);

    if (acceptThread_.joinable())
        acceptThread_.join();

    RioService::StopService();
}

bool RioServerService::Dispatch(uint32 timeoutMs)
{
    return RioService::Dispatch(timeoutMs);
}

bool RioServerService::StartListening()
{
    consoleLogger->Log(Color::Green, L"[Listener] Listening Start\n");

    listenSocket_ = SocketUtils::CreateSocket();
    if (listenSocket_ == INVALID_SOCKET) {
        const int32 errorCode = ::WSAGetLastError();
        const std::wstring message = SocketUtils::GetWinErrorToString(errorCode);
        consoleLogger->Log(Color::Yellow, L"[Listener] CreateSocket failed (code: %d): %s\n", errorCode, message.c_str());
        return false;
    }

    if (SocketUtils::SetReuseAddress(listenSocket_, true) == false) {
        const int32 errorCode = ::WSAGetLastError();
        const std::wstring message = SocketUtils::GetWinErrorToString(errorCode);
        consoleLogger->Log(Color::Yellow, L"[Listener] SetReuseAddress failed (code: %d): %s\n", errorCode, message.c_str());
        return false;
    }

    if (SocketUtils::SetLinger(listenSocket_, 0, 0) == false) {
        const int32 errorCode = ::WSAGetLastError();
        const std::wstring message = SocketUtils::GetWinErrorToString(errorCode);
        consoleLogger->Log(Color::Yellow, L"[Listener] SetLinger failed (code: %d): %s\n", errorCode, message.c_str());
        return false;
    }

    if (SocketUtils::Bind(listenSocket_, GetNetAddress()) == false) {
        const int32 errorCode = ::WSAGetLastError();
        const std::wstring message = SocketUtils::GetWinErrorToString(errorCode);
        consoleLogger->Log(Color::Yellow, L"[Listener] Bind failed (code: %d): %s\n", errorCode, message.c_str());
        return false;
    }

    if (SocketUtils::Listen(listenSocket_, SOMAXCONN) == false) {
        const int32 errorCode = ::WSAGetLastError();
        const std::wstring message = SocketUtils::GetWinErrorToString(errorCode);
        consoleLogger->Log(Color::Yellow, L"[Listener] Listen failed (code: %d): %s\n", errorCode, message.c_str());
        return false;
    }

    return true;
}

void RioServerService::AcceptLoop()
{
    while (acceptRunning_.load()) {
        SOCKADDR_IN sockAddress{};
        int32 addrLen = sizeof(sockAddress);

        SOCKET clientSocket = ::accept(
            listenSocket_,
            reinterpret_cast<SOCKADDR*>(&sockAddress),
            &addrLen);

        if (clientSocket == INVALID_SOCKET) {
            const int32 errorCode = ::WSAGetLastError();
            if (acceptRunning_.load()) {
                const std::wstring message = SocketUtils::GetWinErrorToString(errorCode);
                consoleLogger->Log(Color::Yellow, L"[Listener] accept failed (code: %d): %s\n", errorCode, message.c_str());
            }
            continue;
        }

        if (GetCurrentSessionCount() >= GetMaxSessionCount()) {
            SocketUtils::Close(clientSocket);
            continue;
        }

        std::shared_ptr<SessionBase> session = CreateSession();
        if (session == nullptr) {
            SocketUtils::Close(clientSocket);
            consoleLogger->Log(Color::Red, L"[Listener] Session Creation Error\n");
            continue;
        }

        if (AcceptSession(session, clientSocket, NetAddr(sockAddress)) == false) {
            continue;
        }
    }
}

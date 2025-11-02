#include "pch.h"
#include "SocketUtils.h"

/*----------------
   SocketUtils
----------------*/

LPFN_CONNECTEX			SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX		SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX			SocketUtils::AcceptEx = nullptr;

void SocketUtils::Initialize()
{
	WSADATA wsaData;
	if (0 != ::WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		// TODO: 俊矾 贸府
		// assert(false);
	}

	SOCKET dummySocket = CreateSocket();
	// TODO: 俊矾 贸府 assert
	BindWindowsFunctions(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx));
	BindWindowsFunctions(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx));
	BindWindowsFunctions(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx));
	Close(dummySocket);
}

void SocketUtils::Clean()
{
	::WSACleanup();
}

bool SocketUtils::BindWindowsFunctions(SOCKET socket, GUID guid, LPVOID* fn)
{
	DWORD bytes = 0;

	return (SOCKET_ERROR != ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), fn, sizeof(*fn), OUT &bytes, nullptr, nullptr));
}

SOCKET SocketUtils::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
}

void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET) {
		::closesocket(socket);
	}

	socket = INVALID_SOCKET;
}

bool SocketUtils::SetLinger(SOCKET socket, bool onOff, uint16 linger)
{
	LINGER option;
	option.l_onoff = onOff;
	option.l_linger = linger;
	return SetSocketOption<LINGER>(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	int32 opt = flag ? 1 : 0;
	return SetSocketOption<int32>(socket, SOL_SOCKET, SO_REUSEADDR, opt);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSocketOption<int32>(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSocketOption<int32>(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	int32 opt = flag ? 1 : 0;
	return SetSocketOption<int32>(socket, SOL_SOCKET, TCP_NODELAY, opt);
}

bool SocketUtils::SetUpdateAcceptContext(SOCKET socket, SOCKET listenSocket)
{
	return SetSocketOption<SOCKET>(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}

bool SocketUtils::Bind(SOCKET socket, const NetAddr& addr)
{
	return (SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&addr.GetSockAddrIn()), sizeof(SOCKADDR_IN)));
}

bool SocketUtils::BindAnyAddress(SOCKET socket, uint16 port)
{
	SOCKADDR_IN addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	addr.sin_port = ::htons(port);

	return (SOCKET_ERROR != ::bind(socket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(SOCKADDR_IN)));
}

bool SocketUtils::Listen(SOCKET socket, int32 backlog)
{
	return (SOCKET_ERROR != ::listen(socket, backlog));
}

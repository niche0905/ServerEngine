#include "pch.h"
#include "SocketUtils.h"

/*----------------
   SocketUtils
----------------*/

LPFN_CONNECTEX			SocketUtils::ConnectEx = nullptr;
LPFN_DISCONNECTEX		SocketUtils::DisconnectEx = nullptr;
LPFN_ACCEPTEX			SocketUtils::AcceptEx = nullptr;

RIO_EXTENSION_FUNCTION_TABLE SocketUtils::Rio{};

void SocketUtils::Initialize()
{
	WSADATA wsaData;
	if (0 != ::WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		// TODO: 로그로 출력
		int errorCode = WSAGetLastError();
		std::wstring errMsg = SocketUtils::GetWinErrorToString(errorCode);
		std::wcout << errMsg << std::endl;
		assert(false);
	}

	SOCKET dummySocket = SocketCreator<BackendType::IOCP>::Create();
	if (dummySocket == INVALID_SOCKET) {
		// TODO: 로그로 출력
		int errorCode = WSAGetLastError();
		std::wstring errMsg = SocketUtils::GetWinErrorToString(errorCode);
		std::wcout << errMsg << std::endl;
		assert(false);
	}
	
	// TODO: 에러 처리 assert
	BindWindowsFunctions(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx));
	BindWindowsFunctions(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx));
	BindWindowsFunctions(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx));
	Close(dummySocket);
	
#ifdef USE_RIO
	if (!LoadRioFunctions()) {
		assert(false);
	}
#endif
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
	return SocketCreator<DefaultBackend>::Create();
}

void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET) {
		::closesocket(socket);
	}

	socket = INVALID_SOCKET;
}

bool SocketUtils::LoadRioFunctions()
{
	SOCKET tempSocket =
		::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP,
					nullptr, 0, WSA_FLAG_REGISTERED_IO);

	if (tempSocket == INVALID_SOCKET)
		return false;

	GUID functionTableId = WSAID_MULTIPLE_RIO;
	DWORD bytes = 0;

	const int result = ::WSAIoctl(
		tempSocket,
		SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
		&functionTableId,
		sizeof(functionTableId),
		&Rio,
		sizeof(Rio),
		&bytes,
		nullptr,
		nullptr);

	::closesocket(tempSocket);

	return result != SOCKET_ERROR;
}

std::wstring SocketUtils::GetWinErrorToString(DWORD errorCode)
{
	LPWSTR buffer = nullptr;

	DWORD len = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&buffer),
		0,
		nullptr);

	std::wstring message;

	if (len == 0)
	{
		message = L"Unknown Windows Error";
	}
	else
	{
		message = buffer;
		LocalFree(buffer);
	}

	// trailing \r\n elimination
	while (!message.empty() && (message.back() == L'\n' || message.back() == L'\r'))
		message.pop_back();

	return message;
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

#pragma once
#include "IoBackend.h"
#include "NetworkAddress.h"

/*----------------
   SocketUtils
----------------*/
//
// SocketUtils은 소켓 관련 유틸리티 함수를 제공합니다
// 비동기	 소켓 작업에 필요한 확장 함수 포인터를 관리합니다
//

class SocketUtils
{
public:
	enum class BackendType
	{
		IOCP,
		RIO
	};

public:
#ifdef USE_RIO
	static constexpr BackendType	DefaultBackend = BackendType::RIO;
#else
	static constexpr BackendType	DefaultBackend = BackendType::IOCP;
#endif

public:
	static LPFN_CONNECTEX			ConnectEx;
	static LPFN_DISCONNECTEX		DisconnectEx;
	static LPFN_ACCEPTEX			AcceptEx;

public:
	// windows 소켓 초기화 및 함수	바인딩
	static void Initialize();
	// windows 소켓 정리
	static void Clean();

	// 윈도우즈 소켓 확장 함수 바인딩
	static bool BindWindowsFunctions(SOCKET socket, GUID guid, LPVOID* fn);
	// 소켓 생성
	static SOCKET CreateSocket();
	// 소켓 닫기
	static void Close(SOCKET& socket);

// Windows Error
public:
	static std::wstring GetWinErrorToString(DWORD errorCode);

// 소켓 옵션 설정 함수들
public:
	// Linger 옵션 설정 (소켓 닫기 시 대기 시간)
	static bool SetLinger(SOCKET socket, bool onOff, uint16 linger);
	// 주소 재사용 옵션 설정 (테스트 용이)
	static bool SetReuseAddress(SOCKET socket, bool flag);
	// 수신 버퍼 크기 설정
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	// 송신 버퍼 크기 설정
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	// TCP No Delay 옵션 설정 (Nagle 알고리즘 비활성화)
	static bool SetTcpNoDelay(SOCKET socket, bool flag);
	// AcceptEx 함수 사용을 위한 소켓 설정
	static bool SetUpdateAcceptContext(SOCKET socket, SOCKET listenSocket);

	// 특정 주소에 바인딩
	static bool Bind(SOCKET socket, const NetAddr& addr);
	// 모든 주소에 바인딩
	static bool BindAnyAddress(SOCKET socket, uint16 port);
	// 수신 대기 설정
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);

public:
	template<typename T>
	// 소켓 옵션을 설정하는 템플릿 함수
	static bool SetSocketOption(SOCKET socket, int32 level, int32 optionName, const T& optionValue)
	{
		return (SOCKET_ERROR != ::setsockopt(socket, level, optionName, reinterpret_cast<const char*>(&optionValue), sizeof(T)));
	}

	template<BackendType B>
	struct SocketCreator;

	template<>
	struct SocketCreator<BackendType::IOCP>
	{
		static SOCKET Create()
		{
			return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
		}
	};

	template<>
	struct SocketCreator<BackendType::RIO>
	{
		static SOCKET Create()
		{
			return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_REGISTERED_IO);
		}
	};

};

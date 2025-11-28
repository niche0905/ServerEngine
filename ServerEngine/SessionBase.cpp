#include "pch.h"
#include "SessionBase.h"
#include "NetworkAddress.h"

/*---------------
   SessionBase
---------------*/

SessionBase::SessionBase()
{
	socket_ = SocketUtils::CreateSocket();
}

SessionBase::~SessionBase()
{
	SocketUtils::Close(socket_);
}

void SessionBase::Disconnect(std::wstring_view cause)
{
	if (connected_.exchange(false) == false)
		return;

	// TODO: 로그 남기기(console logger 이용)

	PostDisconnect();
}

void SessionBase::ProcessConnect()
{
	connected_.store(true);

	// TODO: Service에 세션 등록

	OnConnected();

	// Recv waiting
	PostRecv();
}

void SessionBase::ProcessDisconnect()
{
	OnDisconnected();
	// TODO: Service에서 세션 해제
}

void SessionBase::HandleError(std::wstring_view functionName, int32 errorCode)
{
    std::wstring message = SocketUtils::GetWinErrorToString(errorCode);

    // TODO: 여기서 네 Logger 사용하면 됨
    // 예: ConsoleLogger::Error(L"[SessionBase] %s failed: (%d) %s",
    //                           functionName.data(), errorCode, message.c_str());

    // 임시 출력
    wprintf(L"[Session] ERROR in %s (code: %d): %s\n",
        functionName.data(), errorCode, message.c_str());

    switch (errorCode)
    {
    case WSAECONNRESET:
    case WSAECONNABORTED:
    case WSAETIMEDOUT:
        Disconnect(L"HandleError");
        break;

    default:
        break;
    }
}

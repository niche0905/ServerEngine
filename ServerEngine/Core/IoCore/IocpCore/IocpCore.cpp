#include "pch.h"
#include "Core/IoCore/IocpCore/IocpCore.h"
#include "Network/Event/IIoEvent.h"

/*-------------
   IocpCore
-------------*/

class IIoEvent;

IocpCore::IocpCore()
{
	const bool initSucc = Initialize();
	assert(initSucc && "IocpCoreInitialize Failed");
}

IocpCore::~IocpCore()
{
	Terminate();
}

bool IocpCore::Initialize()
{
	iocpHandle_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	return (iocpHandle_ != INVALID_HANDLE_VALUE);
}

void IocpCore::Terminate()
{
	::CloseHandle(iocpHandle_);
}

bool IocpCore::Dispatch(DWORD timeoutMs)
{
	DWORD numOfBytes = 0;
	ULONG_PTR completionKey = 0;
	LPOVERLAPPED overlapped = nullptr;

	BOOL ok = ::GetQueuedCompletionStatus(iocpHandle_, OUT &numOfBytes, OUT &completionKey, OUT &overlapped, timeoutMs);
	
	if (overlapped == nullptr) {
		DWORD error = ::GetLastError();
		if (not ok and error == WAIT_TIMEOUT)
			return false;
		
		// TODO: Error 로그 남기기 (이상 상황)
		return false;
	}
	
	IocpEvent* ev = reinterpret_cast<IocpEvent*>(overlapped);
	
	if (auto owner = ev->GetOwner()) {
		owner->Dispatch(ev, static_cast<int32>(numOfBytes));
	}
	
	return true;
}

bool IocpCore::AttachIoObject(std::shared_ptr<IoObject> ioObject)
{
	return ::CreateIoCompletionPort(ioObject->GetHandle(), iocpHandle_, 0, 0);
}

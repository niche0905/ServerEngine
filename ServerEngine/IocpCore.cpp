#include "pch.h"
#include "IocpCore.h"
#include "IoEvent.h"

/*-------------
   IocpCore
-------------*/

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
	IoEvent* ioEvent = nullptr;

	if (::GetQueuedCompletionStatus(iocpHandle_, OUT &numOfBytes, OUT &completionKey, OUT reinterpret_cast<LPOVERLAPPED*>(&ioEvent), timeoutMs)) {
		if (ioEvent) {
			if (std::shared_ptr<IoObject> owner = ioEvent->owner_) {
				owner->Dispatch(ioEvent, static_cast<int32>(numOfBytes));
			}
		}
	}
	else {

		int32 errorCode = ::WSAGetLastError();
		switch (errorCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			// TODO: Error 로그 남기기
			if (std::shared_ptr<IoObject> owner = ioEvent->owner_)
			{
				owner->Dispatch(ioEvent, static_cast<int32>(numOfBytes));
			}
			break;
		}
	}

	return true;
}

bool IocpCore::AttachIoObject(std::shared_ptr<IoObject> ioObject)
{
	return ::CreateIoCompletionPort(ioObject->GetHandle(), iocpHandle_, 0, 0);
}

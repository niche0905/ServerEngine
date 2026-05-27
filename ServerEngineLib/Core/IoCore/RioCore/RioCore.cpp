#include "pch.h"
#include "RioCore.h"
#include "Network/Event/RIO/RioEvent.h"

/*------------
   RioCore
------------*/

bool RioCore::Initialize()
{
#ifdef USE_RIO
	completionType_.Type = RIO_EVENT_COMPLETION;
	completionType_.Event.EventHandle = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);

	if (completionType_.Event.EventHandle == nullptr)
		return false;

	rioCq_ = SocketUtils::Rio.RIOCreateCompletionQueue(1024, &completionType_);

	if (rioCq_ == RIO_INVALID_CQ)
	{
		::CloseHandle(completionType_.Event.EventHandle);
		completionType_.Event.EventHandle = nullptr;
		return false;
	}

	return true;
#else
	return false;
#endif
}

void RioCore::Terminate()
{
#ifdef USE_RIO
	if (rioCq_ != RIO_INVALID_CQ)
	{
		SocketUtils::Rio.RIOCloseCompletionQueue(rioCq_);
		rioCq_ = RIO_INVALID_CQ;
	}

	if (completionType_.Event.EventHandle != nullptr)
	{
		::CloseHandle(completionType_.Event.EventHandle);
		completionType_.Event.EventHandle = nullptr;
	}
#endif
}

bool RioCore::Dispatch(DWORD timeoutMs)
{
#ifdef USE_RIO
	RIORESULT results[1024];

	ULONG numOfResults = SocketUtils::Rio.RIODequeueCompletion(
		rioCq_,
		results,
		static_cast<ULONG>(std::size(results))
	);

	if (numOfResults == RIO_CORRUPT_CQ)
		return false;

	for (ULONG i = 0; i < numOfResults; ++i)
	{
		RIORESULT& result = results[i];
		RioEvent* ioEvent = reinterpret_cast<RioEvent*>(result.RequestContext);

		if (ioEvent)
		{
			if (std::shared_ptr<IoObject> owner = ioEvent->GetOwner())
			{
				owner->Dispatch(ioEvent, static_cast<int32>(result.BytesTransferred));
			}
		}
	}

	if (numOfResults == 0)
	{
		::WaitForSingleObject(completionType_.Event.EventHandle, timeoutMs);
	}

	return true;
#else
	return false;
#endif
}

bool RioCore::AttachIoObject(std::shared_ptr<IoObject> ioObject)
{
#ifdef USE_RIO
	if (ioObject == nullptr)
		return false;

	SOCKET socket = reinterpret_cast<SOCKET>(ioObject->GetHandle());

	RIO_RQ rq = SocketUtils::Rio.RIOCreateRequestQueue(
		socket,
		1024, 1024,
		1024, 1024,
		rioCq_,
		rioCq_,
		ioObject.get()
	);

	if (rq == RIO_INVALID_RQ)
		return false;

	// TODO:
	// RioSession 같은 객체에 rq 저장 필요
	// ioObject->SetRequestQueue(rq);

	return true;
#else
	return false;
#endif
}

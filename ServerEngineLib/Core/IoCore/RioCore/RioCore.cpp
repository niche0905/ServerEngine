#include "pch.h"

#ifdef USE_RIO

#include "RioCore.h"
#include "Network/Event/RIO/RioEvent.h"
#include "Network/SocketUtils.h"
#include "Network/Session/RIO/IRioObject.h"
#include "Utils/Logger/ConsoleLogger.h"

/*------------
   RioCore
------------*/

RioCore::RioCore()
{
	const bool initSucc = Initialize();
	assert(initSucc && "RioCore Initialize Failed");
}

RioCore::~RioCore()
{
	Terminate();
}

bool RioCore::Initialize()
{
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
}

void RioCore::Terminate()
{
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
}

bool RioCore::Dispatch(DWORD timeoutMs)
{
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
			ioEvent->SetCompletionStatus(result.Status);

			if (std::shared_ptr<IoObject> owner = ioEvent->GetOwner())
			{
				owner->Dispatch(ioEvent, static_cast<int32>(result.BytesTransferred));
			}
		}
	}

	if (numOfResults == 0)
	{
		SocketUtils::Rio.RIONotify(rioCq_);
		::WaitForSingleObject(completionType_.Event.EventHandle, timeoutMs);
	}

	return true;
}

bool RioCore::AttachIoObject(std::shared_ptr<IoObject> ioObject)
{
	if (ioObject == nullptr)
		return false;
	
	auto rioObject = std::dynamic_pointer_cast<IRioObject>(ioObject);
	if (rioObject == nullptr)
		return false;
	
	SOCKET socket = reinterpret_cast<SOCKET>(ioObject->GetHandle());

	RIO_RQ rq = SocketUtils::Rio.RIOCreateRequestQueue(
		socket,
		RioMaxOutstandingReceive,
		RioMaxReceiveDataBuffers,
		RioMaxOutstandingSend,
		RioMaxSendDataBuffers,
		rioCq_,
		rioCq_,
		ioObject.get()
	);

	if (rq == RIO_INVALID_RQ) {
		int32 errorCode = ::WSAGetLastError();
		std::wstring message = SocketUtils::GetWinErrorToString(errorCode);
		consoleLogger->Log(Color::Yellow, L"[Session] ERROR in RioCore::AttachIoObejct (code: %d): %s\n", errorCode, message.c_str());
		return false;
	}
	
	rioObject->SetRequestQueue(rq);

	return true;
}

#endif

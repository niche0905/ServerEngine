#include "pch.h"
#include "RioCore.h"
#include "RioEvent.h"

/*------------
   RioCore
------------*/

bool RioCore::Initialize()
{
	SOCKET tempSocket = ::WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_REGISTERED_IO);
	if (tempSocket == INVALID_SOCKET)
		return false;

	// RIO 함수 테이블 로드
	GUID functionTableId = WSAID_MULTIPLE_RIO;
	DWORD bytes = 0;
	if (::WSAIoctl(tempSocket, SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
		&functionTableId, sizeof(functionTableId),
		&rio_, sizeof(rio_),
		&bytes, nullptr, nullptr) == SOCKET_ERROR
		) {

		::closesocket(tempSocket);
		return false;
	}

	completionType_.Type = RIO_EVENT_COMPLETION;
	completionType_.Event.EventHandle = ::CreateEventW(nullptr, FALSE, FALSE, nullptr);

	if (completionType_.Event.EventHandle == nullptr) {
		::closesocket(tempSocket);
		return false;
	}

	rioCq_ = rio_.RIOCreateCompletionQueue(1024, &completionType_);
	if (rioCq_ == RIO_INVALID_CQ) {
		::CloseHandle(completionType_.Event.EventHandle);
		::closesocket(tempSocket);
		return false;
	}

	::closesocket(tempSocket);
	return true;
}

void RioCore::Terminate()
{
	if (rioCq_ != RIO_INVALID_CQ) {
		rio_.RIOCloseCompletionQueue(rioCq_);
		rioCq_ = RIO_INVALID_CQ;
	}

	if (completionType_.Event.EventHandle != INVALID_HANDLE_VALUE) {
		::CloseHandle(completionType_.Event.EventHandle);
		completionType_.Event.EventHandle = INVALID_HANDLE_VALUE;
	}
}

bool RioCore::Dispatch(DWORD timeoutMs)
{
	RIORESULT results[1024];	// 최대 한번에 받을 수 있는 이벤트 수 (Attach 할 때 설정한 값)
	ULONG numOfResults = rio_.RIODequeueCompletion(rioCq_, results, std::size(results));

	if (numOfResults == RIO_CORRUPT_CQ) {
		// TODO: CQ가 손상된 경우에 대한 처리

		return false;
	}

	for (ULONG i = 0; i < numOfResults; ++i) {
		RIORESULT& result = results[i];
		RioEvent* ioEvent = reinterpret_cast<RioEvent*>(result.RequestContext);
		
		if (ioEvent) {
			if (std::shared_ptr<IoObject> owner = ioEvent->GetOwner()) {
				owner->Dispatch(ioEvent, static_cast<int32>(result.BytesTransferred));
			}
		}
	}

	// 결과가 없으면 Completion Notify 이벤트 대기
	if (numOfResults == 0) {
		::WaitForSingleObject(completionType_.Event.EventHandle, timeoutMs);
	}

	return false;
}

bool RioCore::AttachIoObject(std::shared_ptr<IoObject> ioObject)
{
	// 여기에 들어온 IoObject는 RIO 소켓이어야 합니다 (반드시!!)
	SOCKET socket = reinterpret_cast<SOCKET>(ioObject->GetHandle());

	RIO_RQ rq = rio_.RIOCreateRequestQueue(
		socket,
		1024, 1024,
		1024, 1024,
		rioCq_,
		rioCq_,
		ioObject.get()
	);

	if (rq == RIO_INVALID_RQ)
		return false;

	// TODO: Rio Session으로 변환 후 rq 저장


	return true;
}

bool RioCore::LoadRioFunctions()
{
	return false;
}

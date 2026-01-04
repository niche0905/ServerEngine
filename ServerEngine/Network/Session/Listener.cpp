#include "pch.h"
#include "Listener.h"
#include "Core/Global/CoreGlobal.h"
#include "Utils/Log/ConsoleLogger.h"

/*------------
   Listener
------------*/

Listener::~Listener()
{
	CloseListener();

	for (AcceptEvent* acceptEvent : acceptEvents_) {
		delete acceptEvent;
	}
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(listenSocket_);
}

void Listener::Dispatch(IIoEvent* ioEvent, int32 numOfBytes)
{
	assert(ioEvent->GetType() == IoEventType::Accept);

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(ioEvent);
	ProcessAccept(acceptEvent);
}

bool Listener::StartListening(std::shared_ptr<ServiceBase> service)
{
	service_ = service;
	if (nullptr == service_) {
		return false;
	}
	
	consoleLogger->Log(L"Listeing Start\n");

	listenSocket_ = SocketUtils::CreateSocket();
	if (INVALID_SOCKET == listenSocket_) {
		int errorCode = WSAGetLastError();
		std::wstring errMsg = SocketUtils::GetWinErrorToString(errorCode);
		
		consoleLogger->Log(errMsg.c_str());
		
		return false;
	}

	if (service_->RegisterIoObject(shared_from_this()) == false) {
		return false;
	}

	if (SocketUtils::SetReuseAddress(listenSocket_, true) == false) {
		return false;
	}

	if (SocketUtils::SetLinger(listenSocket_, 0, 0) == false) {
		return false;
	}

	if (SocketUtils::Bind(listenSocket_, service_->GetNetAddress()) == false) {
		return false;
	}

	if (SocketUtils::Listen(listenSocket_, SOMAXCONN) == false) {
		return false;
	}

	const int32 acceptCount = service_->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; ++i) {
		AcceptEvent* acceptEvent = new AcceptEvent();
		acceptEvent->SetOwner(shared_from_this());
		acceptEvents_.push_back(acceptEvent);
		PostAccept(acceptEvent);
	}

	return true;
}

void Listener::CloseListener()
{
	SocketUtils::Close(listenSocket_);
}

void Listener::PostAccept(AcceptEvent* acceptEvent)
{
	acceptEvent->ResetOverlapped();
	
	std::shared_ptr<SessionBase> session = service_->CreateSession();

	// acceptEvent->Init(); <- 이거 제대로 해결된 거 맞나?
	acceptEvent->session_ = session;

	DWORD bytesReceived = 0;
	if (false == SocketUtils::AcceptEx(listenSocket_, session->GetSocket(), session->GetRecvBuffer(), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent))) {
		const int32 errorCode = WSAGetLastError();
		
		if (errorCode != WSA_IO_PENDING) {

			std::wstring errMsg = SocketUtils::GetWinErrorToString(errorCode);
			consoleLogger->Log(errMsg.c_str());
			
			assert(false);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	std::shared_ptr<SessionBase> session = acceptEvent->session_;

	if (false == SocketUtils::SetUpdateAcceptContext(session->GetSocket(), listenSocket_)) {
		PostAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);

	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr)) {
		PostAccept(acceptEvent);
		return;
	}

	session->SetNetAddr(NetAddr(sockAddress));
	session->ProcessConnect();

	PostAccept(acceptEvent);
}

#include "pch.h"
#include "IocpSession.h"

/*---------------
   IocpSession
---------------*/

IocpSession::IocpSession()
{
	socket_ = SocketUtils::CreateSocket();
}

IocpSession::~IocpSession()
{
	SocketUtils::Close(socket_);
}

bool IocpSession::Connect(SOCKET socket)
{
    return PostConnect();
}

void IocpSession::Send(SendBufferRef sendBuffer)
{
	if (IsConnected() == false)
		return;

	bool postSend = false;

	// not lock free, but simple and easy to maintain
	// if not sending_, post send
	{
		std::lock_guard<std::mutex> lock(sendMutex_);

		sendQueue_.push(sendBuffer);

		if (sending_.exchange(true) == false)
			postSend = true;

		if (postSend == true)
			PostSend();
	}
}

void IocpSession::Dispatch(class IIoEvent* ioEvent, int32 numOfBytes)
{
	switch (ioEvent->GetType())
	{
	case IoEventType::Connect:
		ProcessConnect();
		break;
	case IoEventType::Disconnect:
		ProcessDisconnect();
		break;
	case IoEventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case IoEventType::Send:
		ProcessSend(numOfBytes);
		break;
	default:
		assert(false && "PlayerSession::Dispatch - Unknown IoEventType");
		break;
	}
}

byte* IocpSession::GetRecvBuffer()
{
    return recvBuffer_.Data();
}

bool IocpSession::PostConnect()
{
    if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::IocpServerService)
		return false;

	if (SocketUtils::SetReuseAddress(socket_, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(socket_, 0) == false)
		return false;

	connectEvent_.ResetOverlapped();
	connectEvent_.SetOwner(shared_from_this());

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddrIn();
	if (SocketUtils::ConnectEx(socket_, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(SOCKADDR_IN), nullptr, 0, &numOfBytes, &connectEvent_) == false) {
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING) {
			connectEvent_.SetOwner(nullptr);	// Release reference
			return false;
		}
	}

	return true;
}

bool IocpSession::PostDisconnect()
{
	disconnectEvent_.ResetOverlapped();
	disconnectEvent_.SetOwner(shared_from_this());

	if (SocketUtils::DisconnectEx(socket_, &disconnectEvent_, TF_REUSE_SOCKET, 0) == false) {
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING) {
			disconnectEvent_.SetOwner(nullptr);	// Release reference
			return false;
		}
	}

	return true;
}

void IocpSession::PostRecv()
{
	if (IsConnected() == false)
		return;

	recvEvent_.ResetOverlapped();
	recvEvent_.SetOwner(shared_from_this());

	WSABUF wsabuf[2];
	recvBuffer_.PrepareRecv(wsabuf);

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	// TODO: Polymorphic RecvBuffer support
	//	     Currently, only LinearBuffer is supported (3 parameter is Problem)
	if (::WSARecv(socket_, wsabuf, 1, OUT & numOfBytes, OUT & flags, &recvEvent_, nullptr) == SOCKET_ERROR) {
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING) {
			HandleError(L"IocpSession::PostRecv", errorCode);
			recvEvent_.SetOwner(nullptr);	// Release reference
		}
	}
}

void IocpSession::PostSend()
{
	if (IsConnected() == false)
		return;

	sendEvent_.ResetOverlapped();
	sendEvent_.SetOwner(shared_from_this());

	{
		int32 writeSize = 0;
		while (sendQueue_.empty() == false) {
			SendBufferRef sendBuffer = sendQueue_.front();

			writeSize += static_cast<int32>(sendBuffer->Size());
			// TODO: check max write size (+ exception check)

			sendQueue_.pop();
			sendEvent_.sendBuffers_.push_back(sendBuffer);
		}
	}

	std::vector<WSABUF> wsabufs;
	wsabufs.reserve(sendEvent_.sendBuffers_.size());
	for (SendBufferRef sendBuffer : sendEvent_.sendBuffers_) {
	
		WSABUF wsabuf;
		wsabuf.buf = reinterpret_cast<char*>(sendBuffer->Data());
		wsabuf.len = static_cast<ULONG>(sendBuffer->Size());
		wsabufs.push_back(wsabuf);
	}

	DWORD numOfBytes = 0;
	if (::WSASend(socket_, wsabufs.data(), static_cast<DWORD>(wsabufs.size()), OUT & numOfBytes, 0, &sendEvent_, nullptr) == SOCKET_ERROR) {
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != ERROR_IO_PENDING) {
			HandleError(L"IocpSession::PostSend", errorCode);
			sendEvent_.SetOwner(nullptr);	// Release reference

			sendEvent_.sendBuffers_.clear();
			sending_.store(false);
		}
	}
}

void IocpSession::ProcessConnect()
{
	connectEvent_.SetOwner(nullptr);	// Release reference

	connected_.store(true);

	// Session registeration to Service
	GetService()->AddSession(GetSessionRef());

	// on connected event (content override)
	OnConnected();

	// Recv waiting
	PostRecv();
}

void IocpSession::ProcessDisconnect()
{
	disconnectEvent_.SetOwner(nullptr);	// Release reference

	// on disconnected event (content override)
	OnDisconnected();
	GetService()->RemoveSession(GetSessionRef());
}

void IocpSession::ProcessRecv(int32 numOfBytes)
{
	recvEvent_.SetOwner(nullptr);	// Release reference

	if (numOfBytes == 0) {
		Disconnect(L"Remote side disconnected(Recv 0)");
		return;
	}

	if (recvBuffer_.Commit(numOfBytes) == false) {
		Disconnect(L"ProcessRecv invalid Commit");
		return;
	}

	int32 dataSize = static_cast<int32>(recvBuffer_.DataSize());
	
	std::pair<const byte*, size_t> dataSet = recvBuffer_.Peek();

	if (CanPacketProcess(dataSet.first, static_cast<int32>(dataSet.second)) == false) {
		Disconnect(L"ProcessRecv invalid CanPacketProcess");
		return;
	}

	std::vector<byte> processBuffer_;
	processBuffer_.reserve(dataSize);
	recvBuffer_.PeekInto(processBuffer_.data(), dataSize);

	int32 processedBytes = OnRecv(processBuffer_.data(), dataSize);
	if (processedBytes < 0 or dataSize < processedBytes) {
		Disconnect(L"ProcessRecv invalid processedBytes");
		return;
	}

	// remove processed bytes from recv buffer
	recvBuffer_.Consume(processedBytes);

	// continue recv
	PostRecv();
}

void IocpSession::ProcessSend(int32 numOfBytes)
{
	sendEvent_.SetOwner(nullptr);	// Release reference
	sendEvent_.sendBuffers_.clear();

	if (numOfBytes == 0) {
		Disconnect(L"Remote side disconnected(Send 0)");
		return;
	}

	OnSend(numOfBytes);

	std::lock_guard<std::mutex> lock(sendMutex_);
	if (sendQueue_.empty()) {
		sending_.store(false);
	}
	else {
		PostSend();
	}
}

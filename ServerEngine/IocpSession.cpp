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

BYTE* IocpSession::GetRecvBuffer()
{
    return nullptr;
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
	if (SocketUtils::ConnectEx(socket_, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR_IN), nullptr, 0, &numOfBytes, &connectEvent_) == false) {
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

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<CHAR*>(GetRecvBuffer());
	wsaBuf.len = 0; // TODO: recv buffer size (FreeSize())

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (::WSARecv(socket_, &wsaBuf, 1, OUT & numOfBytes, OUT & flags, &recvEvent_, nullptr) == SOCKET_ERROR) {
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
			std::shared_ptr<SendBuffer> sendBuffer = sendQueue_.front();

			writeSize += sendBuffer->Size();
			// TODO: check max write size (+ exception check)

			sendQueue_.pop();
			sendEvent_.sendBuffers_.push_back(sendBuffer);
		}
	}

	std::vector<WSABUF> wsabufs;
	wsabufs.reserve(sendEvent_.sendBuffers_.size());
	for (std::shared_ptr sendBuffer : sendEvent_.sendBuffers_) {
	
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

// TODO: ProcessConnect, ProcessDisconnect thinking...

void IocpSession::ProcessRecv(int32 numOfBytes)
{
	recvEvent_.SetOwner(nullptr);	// Release reference

	if (numOfBytes == 0) {
		Disconnect(L"Remote side disconnected(Recv 0)");
		return;
	}

	// TODO: recv buffer thinking (polymorphic buffer?)
	
}

void IocpSession::ProcessSend(int32 numOfBytes)
{
}

void IocpSession::OnConnected()
{
}

void IocpSession::OnDisconnected()
{
}

void IocpSession::OnRecvPacket(BYTE* buffer, int32 len)
{
}

void IocpSession::OnSend(int32 len)
{
}

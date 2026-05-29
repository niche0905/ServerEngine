#include "pch.h"
#include "RioSession.h"
#include "Network/Buffer/RioBuffer/RioSendBuffer.h"

/*--------------
   RioSession
--------------*/

RioSession::RioSession()
   : recvBuffer_(SessionBase::BUFFER_SIZE)
{
}

RioSession::~RioSession()
{
   recvBuffer_.Unregister();
}

bool RioSession::Connect(SOCKET socket)
{
   socket_ = socket;
   return PostConnect();
}

void RioSession::Send(std::shared_ptr<RioSendBuffer> sendBuffer)
{
   if (IsConnected() == false)
      return;

   if (sendBuffer == nullptr || sendBuffer->WriteSize() <= 0)
      return;

   bool postSend = false;

   {
      std::lock_guard<std::mutex> lock(sendMutex_);

      sendQueue_.push(sendBuffer);

      if (sending_.exchange(true) == false)
         postSend = true;

      if (postSend)
         PostSend();
   }
}

void RioSession::Dispatch(class IIoEvent* ioEvent, int32 numOfBytes)
{
   switch (ioEvent->GetType())
   {
   case IoEventType::Recv:
      ProcessRecv(numOfBytes);
      break;

   case IoEventType::Send:
      ProcessSend(numOfBytes);
      break;

   default:
      break;
   }
}

byte* RioSession::GetRecvBuffer()
{
   return recvBuffer_.Data();
}

bool RioSession::PostConnect()
{
   if (IsConnected())
      return false;

   if (rio_ == nullptr)
      return false;

   if (rq_ == RIO_INVALID_RQ)
      return false;

   if (recvBuffer_.IsRegistered() == false)
   {
      if (recvBuffer_.Register(rio_) == false)
         return false;
   }

   ProcessConnect();
   return true;
}

bool RioSession::PostDisconnect()
{
   ProcessDisconnect();
   return true;
}

void RioSession::PostRecv()
{
   if (IsConnected() == false)
      return;

   if (rio_ == nullptr || rq_ == RIO_INVALID_RQ)
      return;

   if (recvBuffer_.FreeSize() == 0) {
      Disconnect(L"RioSession::PostRecv FreeSize 0");
      return;
   }

   recvEvent_.SetOwner(shared_from_this());

   recvBuffer_.PrepareWrite();
   RIO_BUF rioBuf = recvBuffer_.MakeRecvRioBuf();

   if (rio_->RIOReceive(
       rq_,
       &rioBuf,
       1,
       0,
       reinterpret_cast<PVOID>(&recvEvent_)) == FALSE)
   {
      const int32 errorCode = ::WSAGetLastError();
      recvEvent_.SetOwner(nullptr);
      HandleError(L"RioSession::PostRecv", errorCode);
   }
}

void RioSession::PostSend()
{
   if (IsConnected() == false)
      return;

   if (rio_ == nullptr || rq_ == RIO_INVALID_RQ)
      return;

   sendEvent_.SetOwner(shared_from_this());
   sendEvent_.sendBuffers_.clear();

   std::vector<RIO_BUF> rioBufs;
   int32 totalWriteSize = 0;

   while (sendQueue_.empty() == false) {
      std::shared_ptr<RioSendBuffer> sendBuffer = sendQueue_.front();
      sendQueue_.pop();

      if (sendBuffer == nullptr)
         continue;

      if (sendBuffer->WriteSize() <= 0)
         continue;

      totalWriteSize += sendBuffer->WriteSize();

      sendEvent_.sendBuffers_.push_back(sendBuffer);
      rioBufs.push_back(sendBuffer->MakeRioBuf());
   }

   if (rioBufs.empty()) {
      sendEvent_.SetOwner(nullptr);
      sending_.store(false);
      return;
   }

   if (rio_->RIOSend(
      rq_,
      rioBufs.data(),
      static_cast<ULONG>(rioBufs.size()),
      0,
      reinterpret_cast<PVOID>(&sendEvent_)) == FALSE)
   {
      const int32 errorCode = ::WSAGetLastError();

      sendEvent_.SetOwner(nullptr);
      sendEvent_.sendBuffers_.clear();
      sending_.store(false);

      HandleError(L"RioSession::PostSend", errorCode);
   }
}

void RioSession::ProcessConnect()
{
   connected_.store(true);

   GetService()->AddSession(GetSessionRef());

   OnConnected();

   PostRecv();
}

void RioSession::ProcessDisconnect()
{
   recvBuffer_.Unregister();

   sendEvent_.SetOwner(nullptr);
   recvEvent_.SetOwner(nullptr);

   {
      std::lock_guard<std::mutex> lock(sendMutex_);

      while (sendQueue_.empty() == false)
         sendQueue_.pop();

      sendEvent_.sendBuffers_.clear();
      sending_.store(false);
   }

   OnDisconnected();

   if (auto service = GetService())
      service->RemoveSession(GetSessionRef());
}

void RioSession::ProcessRecv(int32 numOfBytes)
{
   recvEvent_.SetOwner(nullptr);

   if (numOfBytes <= 0)
   {
      Disconnect(L"Recv 0");
      return;
   }

   // RIOReceive가 recvBuffer_.Data() + WritePos()에 직접 써줬으므로
   // 여기서는 writePos_, size_만 증가시키면 된다.
   if (recvBuffer_.Commit(static_cast<size_t>(numOfBytes)) == false)
   {
      Disconnect(L"RecvBuffer Commit Failed");
      return;
   }

   const int32 dataSize = static_cast<int32>(recvBuffer_.DataSize());

   const int32 processLen = OnRecv(
      recvBuffer_.Data() + recvBuffer_.ReadPos(),
      dataSize);

   if (processLen < 0 || processLen > dataSize)
   {
      Disconnect(L"Packet Error");
      return;
   }

   recvBuffer_.Consume(static_cast<size_t>(processLen));

   PostRecv();
}

void RioSession::ProcessSend(int32 numOfBytes)
{
   sendEvent_.SetOwner(nullptr);
   sendEvent_.sendBuffers_.clear();

   if (numOfBytes == 0)
   {
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

#include "pch.h"

#ifdef USE_RIO

#include "RioSession.h"
#include "Network/Buffer/RioBuffer/RioSendBuffer.h"

/*--------------
   RioSession
--------------*/

RioSession::RioSession()
   : recvBuffer_(SessionBase::BUFFER_SIZE)
{
   sendEvent_.sendBuffers_.reserve(RioMaxSendDataBuffers);
   sendEvent_.rioBuffers_.reserve(RioMaxSendDataBuffers);
}

RioSession::~RioSession()
{
   assert(recvPending_.load() == false);
   assert(sendPending_.load() == false);
   recvBuffer_.Unregister();
}

bool RioSession::Connect(SOCKET socket)
{
   socket_ = socket;
   return PostConnect();
}

void RioSession::Send(std::shared_ptr<RioSendBuffer> sendBuffer)
{
   if (IsConnected() == false || closing_.load())
      return;

   if (sendBuffer == nullptr || sendBuffer->WriteSize() <= 0)
      return;

   bool postSend = false;

   {
      std::lock_guard<std::mutex> lock(sendMutex_);

      sendQueue_.push(sendBuffer);

      if (sending_.exchange(true) == false)
         postSend = true;
   }

   if (postSend)
      PostSend();
}

void RioSession::Dispatch(class IIoEvent* ioEvent, int32 numOfBytes)
{
   RioEvent* rioEvent = static_cast<RioEvent*>(ioEvent);
   const LONG completionStatus = rioEvent->GetCompletionStatus();

   completionDispatchCount_.fetch_add(1);

   switch (ioEvent->GetType())
   {
   case IoEventType::Recv:
      recvPending_.store(false);

      if (completionStatus != NO_ERROR)
      {
         recvEvent_.SetOwner(nullptr);
         if (closing_.load() == false)
            HandleRioCompletionError(L"RIOReceive", completionStatus);
      }
      else if (closing_.load())
      {
         recvEvent_.SetOwner(nullptr);
      }
      else
      {
         ProcessRecv(numOfBytes);
      }
      break;

   case IoEventType::Send:
      sendPending_.store(false);

      if (completionStatus != NO_ERROR)
      {
         sendEvent_.SetOwner(nullptr);
         if (closing_.load() == false)
            HandleRioCompletionError(L"RIOSend", completionStatus);
      }
      else if (closing_.load())
      {
         sendEvent_.SetOwner(nullptr);
      }
      else
      {
         ProcessSend(numOfBytes);
      }
      break;

   default:
      break;
   }

   completionDispatchCount_.fetch_sub(1);
   TryFinalizeDisconnect();
}

byte* RioSession::GetRecvBuffer()
{
   return recvBuffer_.Data();
}

bool RioSession::PrepareForConnectedIo()
{
   if (rq_ == RIO_INVALID_RQ)
      return false;

   if (recvBuffer_.IsRegistered())
      return true;

   return recvBuffer_.Register(&SocketUtils::Rio);
}

bool RioSession::PostConnect()
{
   if (IsConnected())
      return false;

   if (PrepareForConnectedIo() == false)
      return false;

   ProcessConnect();
   return true;
}

bool RioSession::PostDisconnect()
{
   closing_.store(true);

   {
      std::lock_guard<std::mutex> ioLock(ioStateMutex_);
      SocketUtils::Close(socket_);
   }

   ProcessDisconnect();
   return true;
}

void RioSession::PostRecv()
{
   int32 errorCode = NO_ERROR;

   {
      std::lock_guard<std::mutex> ioLock(ioStateMutex_);

      if (IsConnected() == false || closing_.load())
         return;

      if (rq_ == RIO_INVALID_RQ)
         return;

      if (recvBuffer_.FreeSize() == 0) {
         errorCode = WSAENOBUFS;
      }
      else
      {
         recvEvent_.SetOwner(shared_from_this());

         recvBuffer_.PrepareWrite();
         RIO_BUF rioBuf = recvBuffer_.MakeRecvRioBuf();
         recvPending_.store(true);

         if (SocketUtils::Rio.RIOReceive(
             rq_,
             &rioBuf,
             1,
             0,
             reinterpret_cast<PVOID>(&recvEvent_)) == FALSE)
         {
            errorCode = ::WSAGetLastError();
            recvPending_.store(false);
            recvEvent_.SetOwner(nullptr);
         }
      }
   }

   if (errorCode != NO_ERROR)
      HandleRioCompletionError(L"RIOReceive post", errorCode);
}

void RioSession::PostSend()
{
   int32 errorCode = NO_ERROR;

   {
      std::lock_guard<std::mutex> ioLock(ioStateMutex_);
      std::lock_guard<std::mutex> sendLock(sendMutex_);

      if (IsConnected() == false || closing_.load() || rq_ == RIO_INVALID_RQ) {
         sending_.store(false);
         return;
      }

      sendEvent_.SetOwner(shared_from_this());
      sendEvent_.sendBuffers_.clear();
      sendEvent_.rioBuffers_.clear();

      while (sendQueue_.empty() == false &&
             sendEvent_.rioBuffers_.size() < RioMaxSendDataBuffers) {
         std::shared_ptr<RioSendBuffer> sendBuffer = sendQueue_.front();
         sendQueue_.pop();

         if (sendBuffer == nullptr || sendBuffer->WriteSize() <= 0)
            continue;

         sendEvent_.sendBuffers_.push_back(sendBuffer);
         sendEvent_.rioBuffers_.push_back(sendBuffer->MakeRioBuf());
      }

      if (sendEvent_.rioBuffers_.empty()) {
         sendEvent_.SetOwner(nullptr);
         sending_.store(false);
         return;
      }

      sendPending_.store(true);

      if (SocketUtils::Rio.RIOSend(
         rq_,
         sendEvent_.rioBuffers_.data(),
         static_cast<ULONG>(sendEvent_.rioBuffers_.size()),
         0,
         reinterpret_cast<PVOID>(&sendEvent_)) == FALSE)
      {
         errorCode = ::WSAGetLastError();
         sendPending_.store(false);
         sendEvent_.SetOwner(nullptr);
         sending_.store(false);
      }
   }

   if (errorCode != NO_ERROR)
      HandleRioCompletionError(L"RIOSend post", errorCode);
}

void RioSession::ProcessConnect()
{
   connected_.store(true);

   if (GetService()->AddSession(GetSessionRef()) == false) {
      Disconnect(L"Service stopping");
      return;
   }

   OnConnected();

   PostRecv();
}

void RioSession::ProcessDisconnect()
{
   TryFinalizeDisconnect();
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
   sendEvent_.rioBuffers_.clear();

   if (numOfBytes == 0)
   {
      Disconnect(L"Remote side disconnected(Send 0)");
      return;
   }

   OnSend(numOfBytes);

   bool postNextSend = false;

   {
      std::lock_guard<std::mutex> lock(sendMutex_);

      if (sendQueue_.empty()) {
         sending_.store(false);
      }
      else {
         postNextSend = true;
      }
   }

   if (postNextSend)
      PostSend();
}

void RioSession::HandleRioCompletionError(std::wstring_view operation, LONG status)
{
   HandleError(operation, static_cast<int32>(status));
   Disconnect(operation);
}

void RioSession::TryFinalizeDisconnect()
{
   if (closing_.load() == false)
      return;

   std::unique_lock<std::mutex> ioLock(ioStateMutex_);

   if (recvPending_.load() || sendPending_.load() || completionDispatchCount_.load() != 0)
      return;

   bool expected = false;
   if (disconnectFinalized_.compare_exchange_strong(expected, true) == false)
      return;

   recvEvent_.SetOwner(nullptr);
   sendEvent_.SetOwner(nullptr);
   recvBuffer_.Unregister();
   rq_ = RIO_INVALID_RQ;

   {
      std::lock_guard<std::mutex> sendLock(sendMutex_);

      while (sendQueue_.empty() == false)
         sendQueue_.pop();

      sendEvent_.sendBuffers_.clear();
      sendEvent_.rioBuffers_.clear();
      sending_.store(false);
   }

   ioLock.unlock();

   if (activeRegistered_.exchange(false)) {
      OnDisconnected();

      if (auto service = GetService())
         service->RemoveSession(GetSessionRef());
   }
}

#endif

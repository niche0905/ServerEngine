#include "pch.h"
#include "RioBufferPool.h"

#include "RioSendBuffer.h"

/*-----------------
   RioBufferPool
-----------------*/

RioBufferPool::~RioBufferPool()
{
   Clear();
}

bool RioBufferPool::Init(RIO_EXTENSION_FUNCTION_TABLE* rio, uint32 blockSize, uint32 blockCount)
{
   if (initialized_)
      return false;
   
   if (rio == nullptr)
      return false;
   
   if (blockSize == 0 || blockCount == 0)
      return false;
   
   rio_ = rio;
   blockSize_ = blockSize;
   blockCount_ = blockCount;
   totalSize_ = static_cast<DWORD>(blockSize_ * blockCount_);
   
   ownerThreadId_ = std::this_thread::get_id();
   
   memory_ = reinterpret_cast<byte*>(::VirtualAlloc(nullptr, totalSize_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
   
   if (memory_ == nullptr)
      return false;
   
   bufferId_ = rio_->RIORegisterBuffer(reinterpret_cast<PCHAR>(memory_), totalSize_);
   
   if (bufferId_ == RIO_INVALID_BUFFERID) {
      ::VirtualFree(memory_, 0, MEM_RELEASE);
      memory_ = nullptr;
      return false;
   }
   
   buffers_.reserve(blockCount_);
   localFreeList_.reserve(blockCount_);
   
   for (uint32 i = 0; i < blockCount_; ++i) {
      auto buffer = std::make_unique<RioSendBuffer>();
      
      buffer->ownerPool_ = this;
      buffer->bufferId_ = bufferId_;
      buffer->blockIndex_ = i;
      buffer->offset_ = i * blockSize_;
      buffer->capacity_ = blockSize_;
      buffer->buffer_ = memory_ + buffer->offset_;
      buffer->writeSize_ = 0;
      
      localFreeList_.push_back(buffer.get());
      buffers_.push_back(std::move(buffer));
   }
   
   initialized_ = true;
   return true;
}

void RioBufferPool::Clear()
{
   if (!initialized_)
      return;
   
   initialized_ = false;
   
   {
      std::lock_guard lock(remoteMutex_);
      while (!remoteFreeQueue_.empty()) {
         remoteFreeQueue_.pop();
      }
   }
   
   localFreeList_.clear();
   buffers_.clear();
   
   if (rio_ != nullptr and bufferId_ != RIO_INVALID_BUFFERID) {
      rio_->RIODeregisterBuffer(bufferId_);
      bufferId_ = RIO_INVALID_BUFFERID;
   }
   
   if (memory_ != nullptr) {
      ::VirtualFree(memory_, 0, MEM_RELEASE);
      memory_ = nullptr;
   }
   
   rio_ = nullptr;
   totalSize_ = 0;
   blockSize_ = 0;
   blockCount_ = 0;
}

std::shared_ptr<RioSendBuffer> RioBufferPool::Pop()
{
   DrainRemoteFrees();
   
   RioSendBuffer* raw = PopInternal();
   if (raw == nullptr)
      return nullptr;
   
   raw->Reset();
   
   return std::shared_ptr<RioSendBuffer>(raw, [](RioSendBuffer* buffer)
   {
      if (buffer != nullptr and buffer->ownerPool_ == nullptr)
      {
         buffer->ownerPool_->Push(buffer);
      }
   });
}

void RioBufferPool::Push(RioSendBuffer* buffer)
{
   if (buffer == nullptr)
      return;
   
   buffer->Reset();
   
   if (IsOwnerThread()) {
      PushLocal(buffer);
   }
   else {
      PushRemote(buffer);
   }
}

void RioBufferPool::DrainRemoteFrees()
{
   if (!IsOwnerThread())
      return;
   
   std::lock_guard lock(remoteMutex_);

   while (!remoteFreeQueue_.empty()) {
      RioSendBuffer* buffer = remoteFreeQueue_.front();
      remoteFreeQueue_.pop();
      
      localFreeList_.push_back(buffer);
   }
}

RioSendBuffer* RioBufferPool::PopInternal()
{
   if (localFreeList_.empty())
      return nullptr;
   
   RioSendBuffer* buffer = localFreeList_.back();
   localFreeList_.pop_back();
   
   return buffer;
}

void RioBufferPool::PushLocal(RioSendBuffer* buffer)
{
   localFreeList_.push_back(buffer);
}

void RioBufferPool::PushRemote(RioSendBuffer* buffer)
{
   std::lock_guard lock(remoteMutex_);
   remoteFreeQueue_.push(buffer);
}

bool RioBufferPool::IsOwnerThread() const
{
   return std::this_thread::get_id() == ownerThreadId_;
}

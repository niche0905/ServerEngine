#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <atomic>
#include <tbb/concurrent_queue.h>

class RioSendBuffer;

/*-----------------
   RioBufferPool
-----------------*/
//
// RioBufferPool는 Windows의 RIO(Registered I/O) 모델에서 사용되는 버퍼 풀을 관리하는 클래스입니다.
//

class RioBufferPool
{
public:
    RioBufferPool() = default;
    ~RioBufferPool();
    
public:
    bool Init(RIO_EXTENSION_FUNCTION_TABLE* rio, uint32 blockSize, uint32 blockCount);
    
    void Clear();
    
    std::shared_ptr<RioSendBuffer> Pop();
    
    void Push(RioSendBuffer* buffer);
    
    void DrainRemoteFrees();
    
    uint32 GetBlockSize() const { return blockSize_; }
    uint32 GetBlockCount() const { return blockCount_; }
    
private:
    RioSendBuffer* PopInternal();
    
    void PushLocal(RioSendBuffer* buffer);
    void PushRemote(RioSendBuffer* buffer);
    
    bool IsOwnerThread() const;
    
private:
    RIO_EXTENSION_FUNCTION_TABLE* rio_ = nullptr;
    
    byte* memory_ = nullptr;
    DWORD totalSize_ = 0;
    
    uint32 blockSize_ = 0;
    uint32 blockCount_ = 0;
    
    RIO_BUFFERID bufferId_ = RIO_INVALID_BUFFERID;
    
    std::vector<std::unique_ptr<RioSendBuffer>> buffers_;
    std::vector<RioSendBuffer*> localFreeList_;
    
    tbb::concurrent_queue<RioSendBuffer*> remoteFreeQueue_;
    
    std::thread::id ownerThreadId_;
    
    std::atomic<bool> initialized_ = false;
    
};

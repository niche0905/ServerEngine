#pragma once
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <vector>
#include <memory>
#include <atomic>
#include <tbb/concurrent_queue.h>
#include "Network/Buffer/RioBuffer/RioSendBuffer.h"

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
    
    bool IsInitialized() const { return initialized_.load(); }
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

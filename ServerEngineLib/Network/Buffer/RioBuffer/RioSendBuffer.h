#pragma once

class RioBufferPool;

/*-----------------
   RioSendBuffer
-----------------*/
//
// RioSendBuffer는 RIO를 사용하여 데이터를 전송하기 위한 버퍼를 나타냅니다.
//

class RioSendBuffer
{
private:
    friend class RioBufferPool;
    
public:
    RioSendBuffer() = default;
    ~RioSendBuffer() = default;
    
public:
    byte* Buffer() const { return buffer_; }
    int32 Capacity() const { return static_cast<int32>(capacity_); }
    int32 WriteSize() const { return writeSize_; }
    
    void Close(int32 writeSize)
    {
        assert(writeSize >= 0);
        assert(writeSize <= static_cast<int32>(capacity_));
        writeSize_ = writeSize;
    }
    
    RIO_BUF MakeRioBuf() const
    {
        return RIO_BUF
        {
            .BufferId = bufferId_,
            .Offset = offset_,
            .Length = static_cast<ULONG>(writeSize_)
        };
    }
    
    void Reset()
    {
        writeSize_ = 0;
    }
    
private:
    RioBufferPool* ownerPool_ = nullptr;
    
    byte* buffer_ = nullptr;
    ULONG offset_ = 0;
    ULONG capacity_ = 0;
    int32 writeSize_ = 0;
    
    RIO_BUFFERID bufferId_ = RIO_INVALID_BUFFERID;
    
    uint32 blockIndex_ = 0;
    
};

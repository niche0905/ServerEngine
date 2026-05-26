#pragma once
#include "Network/Buffer/LinearBuffer/LinearBuffer.h"

class RioRecvBuffer : public LinearBuffer
{
public:
    explicit RioRecvBuffer(uint32 capacity)
        : LinearBuffer(capacity)
    {
    }
    
    ~RioRecvBuffer() = default;
    
public:
    bool Register(RIO_EXTENSION_FUNCTION_TABLE* rio)
    {
        if (rio == nullptr)
            return false;
        
        if (bufferId_ != RIO_INVALID_BUFFERID)
            return false;   // 이미 등록된 버퍼가 존재하는 경우
        
        rio_ = rio;
        
        bufferId_ = rio_->RIORegisterBuffer(reinterpret_cast<PCHAR>(Data()), static_cast<DWORD>(Capacity()));
        
        return bufferId_ != RIO_INVALID_BUFFERID;
    }
    
    void Unregister()
    {
        if (rio_ != nullptr and bufferId_ != RIO_INVALID_BUFFERID)
        {
            rio_->RIODeregisterBuffer(bufferId_);
            bufferId_ = RIO_INVALID_BUFFERID;
        }
        
        rio_ = nullptr;
    }
    
    bool IsRegistered() const
    {
        return bufferId_ != RIO_INVALID_BUFFERID;
    }
    
    RIO_BUF MakeRecvRioBuf() const
    {
        return RIO_BUF{
            .BufferId = bufferId_,
            .Offset = static_cast<DWORD>(WritePos()),
            .Length = static_cast<DWORD>(FreeSize()),
        };
    }
    
private:
    RIO_EXTENSION_FUNCTION_TABLE* rio_ = nullptr;
    RIO_BUFFERID bufferId_ = RIO_INVALID_BUFFERID;
    
};

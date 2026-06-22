#pragma once
#include "Network/Session/PacketSession.h"
#include "IRioObject.h"
#include "Network/Buffer/RioBuffer/RioRecvBuffer.h"
#include "Network/Event/RIO/RioIoEvent.h"

/*--------------
   RioSession
--------------*/
//
// RioSession는 Windows의 RIO(Registered I/O) API를 활용하여 네트워크 통신을 처리하는 세션 클래스입니다.
//

class RioSession : public PacketSession, public  IRioObject
{
public:
    RioSession();
    virtual ~RioSession();
    
// Architecture interface
public:
    virtual bool Connect(SOCKET socket) override;
    void Send(std::shared_ptr<RioSendBuffer> sendBuffer);
    
	virtual void Dispatch(class IIoEvent* ioEvent, int32 numOfBytes = 0) override;
    
// Network interface
public:
    virtual byte* GetRecvBuffer() override;
    
// Event interface
protected:
	virtual bool PrepareForConnectedIo() override final;

    // post is to request
    virtual bool PostConnect() override final;
    virtual bool PostDisconnect() override final;
    virtual void PostRecv() override final;
    virtual void PostSend() override final;

    // process is to complete
    virtual void ProcessConnect() override;
    virtual void ProcessDisconnect() override;
    virtual void ProcessRecv(int32 numOfBytes) override	;
    virtual void ProcessSend(int32 numOfBytes) override;

// Rio object interface
public:
    virtual void SetRequestQueue(RIO_RQ rq) override { rq_ = rq; }
    virtual RIO_RQ GetRequestQueue() const override { return rq_; }
    
private:
    RIO_RQ rq_ = RIO_INVALID_RQ;

    RioRecvBuffer recvBuffer_;
    RioRecvEvent recvEvent_;
    RioSendEvent sendEvent_;

    std::queue<std::shared_ptr<RioSendBuffer>> sendQueue_;
    std::mutex sendMutex_;
    std::atomic<bool> sending_ = false;
    
};

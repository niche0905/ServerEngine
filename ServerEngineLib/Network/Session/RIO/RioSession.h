#pragma once
#include "Network/Session/PacketSession.h"
#include "IRioObject.h"
#include "Network/Buffer/RioBuffer/RioRecvBuffer.h"
#include "Network/Event/RIO/RioIoEvent.h"

class RioSession : public PacketSession, public  IRioObject
{
public:
    virtual bool Connect(SOCKET socket) override;
    void Send(std::shared_ptr<RioSendBuffer> sendBuffer);

    void SetRequestQueue(RIO_RQ rq) override { rq_ = rq; }
    RIO_RQ GetRequestQueue() const override { return rq_; }
    
private:
    RIO_RQ rq_ = RIO_INVALID_RQ;

    RioRecvBuffer recvBuffer_;
    RioRecvEvent recvEvent_;
    RioSendEvent sendEvent_;

    std::queue<std::shared_ptr<RioSendBuffer>> sendQueue_;
    std::mutex sendMutex_;
    std::atomic<bool> sending_ = false;
    
};

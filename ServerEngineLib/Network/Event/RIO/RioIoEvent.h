#pragma once
#include "RioEvent.h"

class RioSendBuffer;

/*----------------
   RioRecvEvent
----------------*/

class RioRecvEvent : public RioEvent
{
public:
    RioRecvEvent()
        : RioEvent(IoEventType::Recv)
    {

    }
    
};

/*----------------
   RioSendEvent
----------------*/

class RioSendEvent : public RioEvent
{
public:
    RioSendEvent()
        : RioEvent(IoEventType::Send)
    {

    }
    
public:
    std::vector<std::shared_ptr<RioSendBuffer>> sendBuffers_;
    std::vector<RIO_BUF> rioBuffers_;
    
};

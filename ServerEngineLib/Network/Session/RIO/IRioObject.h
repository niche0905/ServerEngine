#pragma once

class IRioObject
{
public:
    virtual ~IRioObject() = default;

    virtual void SetRequestQueue(RIO_RQ rq) = 0;
    virtual RIO_RQ GetRequestQueue() const = 0;
    
};

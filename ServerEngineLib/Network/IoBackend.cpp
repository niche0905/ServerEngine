#include "pch.h"
#include "IoBackend.h"

#ifdef USE_RIO
#include "Core/Thread/ThreadLocalStorage.h"
#include "Network/Buffer/RioBuffer/RioBufferPool.h"
#include "Network/Buffer/RioBuffer/RioSendBuffer.h"
#else
#include "Network/Buffer/SendBuffer.h"
#endif

SendBufferRef MakeNetworkSendBuffer(int32 packetSize)
{
#ifdef USE_RIO
    SendBufferRef sendBuffer = TLS().rioBufferPool.Pop();

    if (sendBuffer == nullptr)
        return nullptr;

    if (packetSize > sendBuffer->Capacity())
        return nullptr;

    return sendBuffer;
#else
    return std::make_shared<NetworkSendBuffer>(packetSize);
#endif
}
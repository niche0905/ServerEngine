#include "pch.h"
#include "IoBackend.h"

#ifdef USE_RIO
#include "Core/Global/CoreGlobal.h"
#include "Core/Thread/ThreadLocalStorage.h"
#include "Network/Buffer/RioBuffer/RioBufferPool.h"
#include "Network/Buffer/RioBuffer/RioSendBuffer.h"
#include "Utils/Logger/ConsoleLogger.h"
#else
#include "Network/Buffer/SendBuffer.h"
#endif

SendBufferRef MakeNetworkSendBuffer(int32 packetSize)
{
#ifdef USE_RIO
    RioBufferPool& pool = TLS().rioBufferPool;

    if (pool.IsInitialized() == false) {
        if (pool.Init(&SocketUtils::Rio, RioSendBlockSize, RioSendBlockCount) == false) {
            if (consoleLogger)
                consoleLogger->Log(Color::Yellow, L"[RIO] SendBufferPool init failed. packetSize=%d blockSize=%u blockCount=%u\n", packetSize, RioSendBlockSize, RioSendBlockCount);
            return nullptr;
        }
    }

    if (packetSize > static_cast<int32>(RioSendBlockSize)) {
        if (consoleLogger)
            consoleLogger->Log(Color::Yellow, L"[RIO] packetSize exceeds send block. packetSize=%d blockSize=%u\n", packetSize, RioSendBlockSize);
        return nullptr;
    }

    SendBufferRef sendBuffer = pool.Pop();

    if (sendBuffer == nullptr) {
        if (consoleLogger)
            consoleLogger->Log(Color::Yellow, L"[RIO] SendBufferPool exhausted. packetSize=%d blockSize=%u blockCount=%u\n", packetSize, RioSendBlockSize, RioSendBlockCount);
        return nullptr;
    }

    return sendBuffer;
#else
    return std::make_shared<NetworkSendBuffer>(packetSize);
#endif
}

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

#define MESSAGE_SIZE 64

enum class OperationType {
    RECV,
    SEND
};

struct RIOContext {
    OperationType op;
    char* buffer;
    RIO_BUF rioBuf;
};

LPFN_RIORECEIVE RIOReceive;
LPFN_RIOSEND RIOSend;
LPFN_RIOREGISTERBUFFER RIORegisterBuffer;
LPFN_RIOCLOSECOMPLETIONQUEUE RIOCloseCompletionQueue;
LPFN_RIOCREATECOMPLETIONQUEUE RIOCreateCompletionQueue;
LPFN_RIOCREATEREQUESTQUEUE RIOCreateRequestQueue;
LPFN_RIODEQUEUECOMPLETION RIODequeueCompletion;

RIO_EXTENSION_FUNCTION_TABLE g_rio = { 0 };

void LoadRIOFunctions(SOCKET s)
{
    GUID funcTableId = WSAID_MULTIPLE_RIO;
    DWORD bytes = 0;

    WSAIoctl(
        s,
        SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
        &funcTableId, sizeof(funcTableId),
        &g_rio, sizeof(g_rio),
        &bytes, NULL, NULL
    );

    RIOReceive = g_rio.RIOReceive;
    RIOSend = g_rio.RIOSend;
    RIORegisterBuffer = g_rio.RIORegisterBuffer;
    RIOCreateCompletionQueue = g_rio.RIOCreateCompletionQueue;
    RIOCreateRequestQueue = g_rio.RIOCreateRequestQueue;
    RIODequeueCompletion = g_rio.RIODequeueCompletion;
    RIOCloseCompletionQueue = g_rio.RIOCloseCompletionQueue;
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
        WSA_FLAG_REGISTERED_IO | WSA_FLAG_OVERLAPPED);
    LoadRIOFunctions(listenSocket);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    LoadRIOFunctions(clientSocket);

    // Buffer 등록
    char buffer[MESSAGE_SIZE] = { 0 };
    RIO_BUFFERID bufferId = RIORegisterBuffer(buffer, MESSAGE_SIZE);
    if (bufferId == RIO_INVALID_BUFFERID) {
        std::cerr << "Failed to register buffer!" << std::endl;
        exit(-1);
    }

    RIO_CQ cq = RIOCreateCompletionQueue(16, NULL);
    if (cq == RIO_INVALID_CQ) {
        std::cerr << "Failed to create CompletionQueue!" << std::endl;
        exit(-1);
    }
    RIO_RQ rq = RIOCreateRequestQueue(clientSocket, 1, 1, 1, 1, cq, cq, NULL);
    if (rq == RIO_INVALID_RQ) {
        std::cerr << "RIOCreateRequestQueue failed: " << WSAGetLastError() << std::endl;
        exit(-1);
    }

    // 수신 요청
    RIOContext recvCtx = {
        OperationType::RECV,
        buffer,
        { bufferId, 0, MESSAGE_SIZE }
    };
    RIOReceive(rq, &recvCtx.rioBuf, 1, 0, &recvCtx);

    RIOContext sendCtx = {
                OperationType::SEND,
                buffer,
                { bufferId, 0, MESSAGE_SIZE }
    };

    while (true) {
        RIORESULT result[1];
        ULONG received = RIODequeueCompletion(cq, result, 1);
        if (received > 0) {
            RIOContext* ctx = reinterpret_cast<RIOContext*>(result[0].RequestContext);

            switch (ctx->op) {
            case OperationType::RECV:
                std::cout << "[Server] Received [" << result[0].BytesTransferred << "]: " << ctx->buffer << std::endl;
                // Echo 보내기
                RIOSend(rq, &sendCtx.rioBuf, 1, 0, &sendCtx); // 같은 context 재사용 가능
                break;

            case OperationType::SEND:
                // 송신 완료 → 아무 작업 안 해도 됨
                std::cout << "[Server] Send complete\n";
                // 다시 수신 등록
                RIOReceive(rq, &recvCtx.rioBuf, 1, 0, &recvCtx);
                break;
            }
        }
    }

    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}

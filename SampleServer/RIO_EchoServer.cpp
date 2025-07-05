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
    // RIORESULT의 RequestContext로 현재 수행한 작업이 어떤 작업인지 구분하기 위해

    OperationType op;
    char* buffer;
    RIO_BUF rioBuf;
};

LPFN_RIORECEIVE     RIORecv;     // RIORecv를 위한 함수 (WSARecv와 유사)
LPFN_RIOSEND        RIOSend;     // RIOSend를 위한 함수 (WSASend와 유사)
LPFN_RIOREGISTERBUFFER RIORegisterBuffer;       // 버퍼를 등록하기 위한 함수
LPFN_RIOCREATECOMPLETIONQUEUE RIOCreateCompletionQueue; // CompletionQueue를 생성하기 위한 함수 (완료 된 것 모아두는 곳)
LPFN_RIOCREATEREQUESTQUEUE RIOCreateRequestQueue;       // RequestQueue를 생성하기 위한 함수 (작업 시켜 놓은 것 모아두는 곳)
LPFN_RIODEQUEUECOMPLETION RIODequeueCompletion;         // CompletionQueue에서 완료된 작업 빼내는 함수

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

    RIORecv = g_rio.RIOReceive;
    RIOSend = g_rio.RIOSend;
    RIORegisterBuffer = g_rio.RIORegisterBuffer;
    RIOCreateCompletionQueue = g_rio.RIOCreateCompletionQueue;
    RIOCreateRequestQueue = g_rio.RIOCreateRequestQueue;
    RIODequeueCompletion = g_rio.RIODequeueCompletion;
}

int main()
{
    // 윈도우 소켓 프로그래밍을 위한 초기화
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 리슨 소켓 준비
    SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0,
        WSA_FLAG_REGISTERED_IO | WSA_FLAG_OVERLAPPED);
    LoadRIOFunctions(listenSocket); // 리슨소켓 RIO에 등록 (Ioct)

    // 서버 주소 및 포트할당
    sockaddr_in serverAddr{};                       // 0으로 초기화 (메모리 Set 안해도 되게끔 구조체 선언과 동시에 초기화)
    serverAddr.sin_family = AF_INET;                // IPv4 사용
    serverAddr.sin_port = htons(9000);              // 포트번호 9000
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 모든 주소에서의 Connect를 Accept 하겠다
    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)); // 리슨 소켓 바인드
    listen(listenSocket, SOMAXCONN);                                // 리슨 시작

    // 클라이언트 소켓 Accept
    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    LoadRIOFunctions(clientSocket); // 클라소켓 RIO에 등록 (Ioct)

    // Buffer 등록
    char buffer[MESSAGE_SIZE] = { 0 };
    RIO_BUFFERID bufferId = RIORegisterBuffer(buffer, MESSAGE_SIZE);    // RIO에서 모든 버퍼는 RIO에 등록 되어야만 한다
    if (bufferId == RIO_INVALID_BUFFERID) {                             // 등록 실패 체크
        std::cerr << "Failed to register buffer!" << std::endl;
        exit(-1);
    }

    RIO_CQ cq = RIOCreateCompletionQueue(16, NULL); // CQ 생성
    if (cq == RIO_INVALID_CQ) {                     // CQ 생성 실패 체크
        std::cerr << "Failed to create CompletionQueue!" << std::endl;
        exit(-1);
    }
    RIO_RQ rq = RIOCreateRequestQueue(clientSocket, 1, 1, 1, 1, cq, cq, NULL);  // RQ 생성
    if (rq == RIO_INVALID_RQ) {                                                 // RQ 생성 실패 체크
        std::cerr << "RIOCreateRequestQueue failed: " << WSAGetLastError() << std::endl;
        exit(-1);
    }

    // 수신 요청
    RIOContext recvCtx = {      // RIORecv 호출 시 사용할 Context
        OperationType::RECV,
        buffer,
        { bufferId, 0, MESSAGE_SIZE }
    };
    RIOContext sendCtx = {      // RIOSend 호출 시 사용할 Context
        OperationType::SEND,
        buffer,
        { bufferId, 0, MESSAGE_SIZE }
    };

    RIORecv(rq, &recvCtx.rioBuf, 1, 0, &recvCtx);   // 비동기 Recv 호출 (RIORecv 호출)
                                                    // WSARecv와 같이 Gather/Scatter를 위한 개수와를 설정하고, flag를 설정할 수 있는 것으로 보인다

    while (true) {
        RIORESULT result[1];    // 완료된 작업에 담겨있는 정보를 받기 위함
        ULONG received = RIODequeueCompletion(cq, result, 1);   // CQ에 완료된 작업 빼내기 (마지막 인자는 몇개씩 빼낼지 개수에 관한 것으로 보인다)
        if (received > 0) { // 완료된 작업이 있다면 (1개 이상 빼냈다면 -> 물론 1개씩 밖에 안꺼내긴 한다만)
            RIOContext* ctx = reinterpret_cast<RIOContext*>(result[0].RequestContext);  // RequestContext의 void*를 우리가 설정한 RIOContext로 형변환

            switch (ctx->op) {  // 완료된 작업 구분
            case OperationType::RECV:   // 비동기 Recv가 완료 된 것이라면 -> 받은 메세지 출력 후 Send 요청
                std::cout << "[Server] Received [" << result[0].BytesTransferred << "]: " << ctx->buffer << std::endl;
                // Echo 보내기
                RIOSend(rq, &sendCtx.rioBuf, 1, 0, &sendCtx); // 같은 context 재사용 가능
                break;

            case OperationType::SEND:   // 비동기 Send가 완료 된 것이라면 -> 로그 출력 후 Recv 요청
                // 송신 완료 → 아무 작업 안 해도 됨
                std::cout << "[Server] Send complete\n";
                // 다시 수신 등록
                RIORecv(rq, &recvCtx.rioBuf, 1, 0, &recvCtx);
                break;
            }
        }
    }

    // 소켓 닫고
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();   // WSAStartUp에 대한 종료 선언

    return 0;
}

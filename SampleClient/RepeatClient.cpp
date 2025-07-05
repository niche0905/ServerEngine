#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

#define MESSAGE_SIZE 64

int main()
{
    std::this_thread::sleep_for(std::chrono::seconds(3));

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));

    const char* msg = "Hello RIO!";
    char recvBuf[MESSAGE_SIZE] = { 0 };

    while (true) {

        send(sock, msg, (int)strlen(msg), 0);
        recv(sock, recvBuf, MESSAGE_SIZE, 0);
        std::cout << "[Client] Received Echo: " << recvBuf << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}

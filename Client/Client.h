#pragma once
#include <iostream>
#include <thread>
#include <string>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include "Common.h"

#pragma comment(lib, "ws2_32.lib")

class Client {
public:
    SOCKET clientSocket = 0;

    Client() { // todo
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed.\n";
            return;
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed.\n";
            WSACleanup();
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);
        serverAddr.sin_port = htons(8080);

        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connection failed.\n";
            closesocket(clientSocket);
            WSACleanup();
            return;
        }
    }

    void sendMessage(const char operationType, const std::string& message) {
        Common::sendChunkedData(clientSocket, operationType, message, 100);
    }

    void receiveMessage() {
        Common::receiveChunkedData(clientSocket);
    }

    ~Client() {
        closesocket(clientSocket);
        WSACleanup();
    }
};


#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <WinSock2.h>
#include "Common.h" // todo: clean up includes

#pragma comment(lib, "ws2_32.lib")

class Server {
public:
    SOCKET serverSocket = 0;
    std::vector<SOCKET> clients;

    Server() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            Common::errorMessage("WSAStartup failed.\n");
            return;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            Common::errorMessage("Socket creation failed.\n");
            WSACleanup();
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8080);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            Common::errorMessage("Bind failed.\n");
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            Common::errorMessage("Listen failed.\n");
            closesocket(serverSocket);
            WSACleanup();
            return;
        }

        std::cout << "Server is listening on port 8080...\n";
    }

    void broadcastMessage(const std::string& message, SOCKET senderSocket, std::mutex& consoleMutex) {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "Client " << senderSocket << ": " << message << std::endl;

        for (SOCKET client : clients) {
            if (client != senderSocket) {
                Common::sendChunkedData(client, 'm', message, 100);
            }
        }
    }

    void handleClient(SOCKET clientSocket, std::mutex& consoleMutex) {
        clients.push_back(clientSocket);

        while (true) {
            char method = Common::receiveOptionType(clientSocket);

            if (method == '-') {
                std::lock_guard<std::mutex> lock(consoleMutex); // todo: implement in other places
                std::cout << "Client " << clientSocket << " disconnected.\n";
                break;
            }
            std::string message = Common::receiveChunkedData(clientSocket);

            broadcastMessage(message, clientSocket, consoleMutex);
        }

        closesocket(clientSocket);
    }

    ~Server() {
        closesocket(serverSocket);
        WSACleanup();
    }
};

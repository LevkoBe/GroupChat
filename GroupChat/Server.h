#pragma once

#include "Common.h"
#include <string>
#include <vector>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

class Server {
public:
    SOCKET serverSocket = 0;
    std::vector<SOCKET> clients;

    Server();

    void broadcastMessage(const std::string& message, SOCKET senderSocket, std::mutex& consoleMutex);

    void handleClient(SOCKET clientSocket, std::mutex& consoleMutex);

    ~Server();
};

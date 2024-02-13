#pragma once
#include <iostream>
#include <string>
#include <Ws2tcpip.h>
#include "Common.h"

#pragma comment(lib, "ws2_32.lib")

class Client {
public:
    SOCKET clientSocket = 0;

    Client();

    bool sendMessage(const char operationType, const std::string& message);

    std::string receiveMessage();

    void receiveHistory();

    void receiveMessages();

    void print(const std::string& output, int numLines = 3);

    void clearLastLine();

    ~Client();
};


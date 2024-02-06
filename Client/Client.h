#pragma once
#include <iostream>
#include <thread>
#include <string>
#include <Ws2tcpip.h>
#include "Common.h"

#pragma comment(lib, "ws2_32.lib")

class Client {
public:
    SOCKET clientSocket = 0;

    Client();

    void sendMessage(const char operationType, const std::string& message);

    void receiveMessage();

    ~Client();
};


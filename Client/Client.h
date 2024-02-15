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
    ~Client();

    // messaging
    bool sendMessage(const char operationType, const std::string& message);

    void receiveMessages();
    std::string receiveMessage();

    void receiveHistory();

    
    // files
    void sendFile();


    // console
    void clearLastLine();
    void print(const std::string& output, int numLines = 3);

};


#pragma once
#include <iostream>
#include <string>
#include <Ws2tcpip.h>
#include "Common.h"

#pragma comment(lib, "ws2_32.lib")

enum State {
    Entering,
    Disconnected,
    AnswerRequired,
    Messaging,
    Waiting,
};

class Client {
public:
    SOCKET clientSocket = 0;
    Client();
    ~Client();

    // Q&A
    bool answerQuestion(State& state);

    // 
    bool enterGroup(State& state);

    // messaging
    bool sendMessage();

    void receiveMessages(State& state);
    std::string receiveMessage();

    void receiveHistory();

    
    // files
    bool sendFile();


    // console
    void clearLastLine();
    void setPalette(int palette = 0);
    void print(const std::string& output="", int numLines = 3, int palette = 0);

};


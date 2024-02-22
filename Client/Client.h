#pragma once
#include <iostream>
#include <string>
#include <cctype>
#include <Ws2tcpip.h>
#include "Common.h"

#pragma comment(lib, "ws2_32.lib")

enum State {
    Messaging, // ok
    Entering,       // registering
    AnswerRequired, // for files
    Waiting,        // waiting for registration
    Disconnected,   // terminate app
};

class Client {
public:
    SOCKET clientSocket = 0;
    Client();
    ~Client();

    // Q&A
    //bool answerQuestion(std::atomic<State>& state);

    // 
    bool enterGroup(std::atomic<State>& state);

    // messaging
    bool sendMessage(std::atomic<State>& state);

    void receiveMessages(std::atomic<State>& state);

    void receiveHistory();

    
    // files
    bool sendFile();


    // console
    void clearLastLine();
    void setPalette(int palette = 0);
    void print(const std::string& output="", int numLines = 3, int palette = 0);

};


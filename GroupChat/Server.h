#pragma once

#include "Common.h"
#include "Room.h"
#include <string>
#include <sstream>
#include <vector>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

class Server {

    std::string invitingMessage(std::string& username);

    bool tryParseInt(const std::string& s, int& result);

    void addUser(User user, Room& room, SOCKET clientSocket);

    void receiveMessages(User& user, std::mutex& consoleMutex);

    Room roomByString(std::string& roomString);
public:
    SOCKET serverSocket = 0;
    std::vector<SOCKET> clients;
    std::vector<Room> rooms;

    Server();

    void broadcastMessage(const std::string& message, SOCKET senderSocket, std::mutex& consoleMutex, Room& room);

    void handleClient(SOCKET clientSocket, std::mutex& consoleMutex);

    User registerClient(SOCKET clientSocket);

    void clientMessaging(SOCKET clientSocket);

    ~Server();
};

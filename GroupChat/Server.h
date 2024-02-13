#pragma once

#include "Common.h"
#include "Room.h"
#include <string>
#include <sstream>
#include <vector>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

class Server {

    // helpers
    std::string invitingMessage(const std::string& username);
    bool tryParseInt(const std::string& s, int& result);
    std::shared_ptr<Room> roomByString(std::string& roomString);

    // registration
    std::shared_ptr<User> registerClient(SOCKET clientSocket);
    void addUser(std::shared_ptr <User> user, std::shared_ptr<Room> room, SOCKET clientSocket);

    std::string askForPassword(SOCKET clientSocket, int index = -1);
    std::string askForUsername(SOCKET clientSocket);
    std::string askForGroupname(SOCKET clientSocket, const std::string& username);

    // messaging
    void receiveMessages(std::shared_ptr<User> user, std::mutex& consoleMutex);
    void broadcastMessage(const Message& message, SOCKET senderSocket, std::mutex& consoleMutex, std::shared_ptr<Room> room);

public:
    SOCKET serverSocket = 0;
    std::vector<SOCKET> clients;
    std::vector<std::shared_ptr<Room>> rooms;

    Server();

    void handleClient(SOCKET clientSocket, std::mutex& consoleMutex);

    ~Server();
};

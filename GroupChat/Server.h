#pragma once

#include "Common.h"
#include "Room.h"
#include <string>
#include <sstream>
#include <vector>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

class Server {

    std::string invitingMessage(const std::string& username);

    bool tryParseInt(const std::string& s, int& result);

    void addUser(std::shared_ptr <User> user, std::shared_ptr<Room> room, SOCKET clientSocket);

    void receiveMessages(std::shared_ptr<User> user, std::mutex& consoleMutex);

    std::shared_ptr<Room> roomByString(std::string& roomString);
public:
    SOCKET serverSocket = 0;
    std::vector<SOCKET> clients;
    std::vector<std::shared_ptr<Room>> rooms;

    Server();

    void broadcastMessage(const Message& message, SOCKET senderSocket, std::mutex& consoleMutex, std::shared_ptr<Room> room);

    void handleClient(SOCKET clientSocket, std::mutex& consoleMutex);

    std::shared_ptr<User> registerClient(SOCKET clientSocket);

    void clientMessaging(SOCKET clientSocket);

    std::string askForPassword(SOCKET clientSocket, int index = -1);

    std::string askForUsername(SOCKET clientSocket);

    std::string askForGroupname(SOCKET clientSocket, const std::string& username);

    ~Server();
};

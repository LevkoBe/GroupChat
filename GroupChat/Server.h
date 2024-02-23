#pragma once

#include <string>
#include "User.h"
#include "Room.h"
#include "Common.h"
#include "Messenger.h"

#pragma comment(lib, "ws2_32.lib")

class Server {

    // helpers
    std::string invitingMessage(const std::string& username);
    bool tryParseInt(const std::string& s, int& result);
    std::shared_ptr<Room> roomByString(std::string& roomString);

    // registration
    std::shared_ptr<Room> joinRoom(std::shared_ptr<User> user);
    std::shared_ptr<Room> newRoom(std::shared_ptr<User> user);

    void addUser(std::shared_ptr <User> user, std::shared_ptr<Room> room, SOCKET clientSocket);

    bool askForPassword(std::shared_ptr<User> user, const std::string& password);
    std::string askForUsername(SOCKET clientSocket);
    std::string askForGroupname(SOCKET clientSocket, const std::string& username);

    // messaging
    void receiveMessages(std::shared_ptr<User> user, std::shared_ptr<Room> room);
    char receiveSignal(std::shared_ptr<User> user);

public:
    SOCKET serverSocket = 0;
    std::vector<SOCKET> clients;
    std::vector<std::shared_ptr<Room>> rooms;
    std::mutex& consoleMutex;
    std::mutex roomsLock;
    std::mutex clientsMutex;
    Messenger messenger;

    Server(std::mutex& consoleMutex);

    void handleClient(SOCKET clientSocket);

    ~Server();
};

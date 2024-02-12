#include "Server.h"
#include "User.h"

Server::Server() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        Common::errorMessage("WSAStartup failed.\n");
        return;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        Common::errorMessage("Socket creation failed.\n");
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        Common::errorMessage("Bind failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        Common::errorMessage("Listen failed.\n");
        closesocket(serverSocket);
        WSACleanup();
        return;
    }

    std::cout << "Server is listening on port 8080...\n";
}

void Server::broadcastMessage(const Message& message, SOCKET senderSocket, std::mutex& consoleMutex, std::shared_ptr<Room> room) {

    room->messageHistory.push_back(message.toStr());
    for (std::shared_ptr<User> user : room->users) {
        SOCKET client = user->clientSocket;
        if (client != senderSocket) {
            Common::sendChunkedData(client, 'm', message.toStr(), 100);
        }
    }
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "Client " << senderSocket << ": " << message << std::endl;
}

std::shared_ptr<Room> Server::roomByString(std::string& roomString) {
    for (auto& room : rooms) {
        if (room->groupName == roomString) {
            return room;
        }
    }
    return nullptr;
}

void Server::handleClient(SOCKET clientSocket, std::mutex& consoleMutex) {
    clients.push_back(clientSocket);
    
    std::shared_ptr<User> user = registerClient(clientSocket); // put the user into a group
    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "Client " << clientSocket << " added to a group.\n";
    }
    receiveMessages(user, consoleMutex);
}

void Server::receiveMessages(std::shared_ptr<User> user, std::mutex& consoleMutex) {
    std::string message;
    std::shared_ptr<Room> room = roomByString(user->room);
    std::unique_ptr<Message> userMessage = std::make_unique<Message>();
    while (true) {
        char method = Common::receiveOptionType(user->clientSocket);
        switch (method) {
        case 'm':
            message = Common::receiveChunkedData(user->clientSocket);
            *userMessage = Message(message, user->username, user->room);
            broadcastMessage(*userMessage, user->clientSocket, consoleMutex, room);
            break;
        case 'f':
            break;
        case 'a':
            break;
        case 'g':
            break;
        case 'x':
            break;
        case '-':
        default:
            std::lock_guard<std::mutex> lock(consoleMutex);
            Common::sendChunkedData(user->clientSocket, '-', "Thank you for being with us!", 100);
            std::cout << "Client " << user->clientSocket << " disconnected.\n";
            closesocket(user->clientSocket);
            return;
            break;
        }
    }
}

std::string Server::invitingMessage(std::string& username) {

    std::string message = "Hi, " + username + "!\n";
    switch (rooms.size()) {
    case 0:
        message += "No available rooms.\n";
        break;
    case 1:
        message += "Available room is:\n";
        break;
    default:
        message += "Available rooms are:\n";
        break;
    }
    for (int i = 0; i < rooms.size(); i++) {
        if (rooms[i]->password != "") {
            message += "[" + std::to_string(i + 1) + ". " + rooms[i]->groupName + "]\n";
        }
        else {
            message += std::to_string(i + 1) + ". " + rooms[i]->groupName + "\n";
        }
    }
    message += "(square brackets represent private groups with a need to provide password)\n";
    message += "If you have list of existing groups, you can join to any of them by just entering their index.\n";
    message += "If you are going to create new group, simply type its name, and hit 'Enter'.\n";

    return message;
}

bool Server::tryParseInt(const std::string& s, int& result) {
    std::istringstream iss(s);
    char leftover;
    if (iss >> result && !(iss >> leftover)) {
        return true;
    }
    return false;
}

void Server::addUser(std::shared_ptr<User> user, std::shared_ptr<Room> room, SOCKET clientSocket) {
    room->users.push_back(user);
    user->room = room->groupName;
    Common::sendChunkedData(user->clientSocket, 'm', ("You've been added successfully to the group '" + user->room + "'.\n"));
    for (auto& message : room->messageHistory) {
        Common::sendChunkedData(clientSocket, 'm', message);
    }
}

std::shared_ptr<User> Server::registerClient(SOCKET clientSocket) {
    Common::sendChunkedData(clientSocket, 'm', "Please, enter your name to connect to the server");
    Common::receiveOptionType(clientSocket);
    std::string username = Common::receiveChunkedData(clientSocket);
    std::shared_ptr<User> user = std::make_shared<User>(username, clientSocket);

    std::string message = invitingMessage(username); // ask for room name
    Common::sendChunkedData(clientSocket, 'm', message);

    Common::receiveOptionType(clientSocket);
    message = Common::receiveChunkedData(clientSocket); // parsing answer
    int index;
    if (!tryParseInt(message, index) || index-- > rooms.size() || index < 0) {
        index = rooms.size();
        std::shared_ptr<Room> room = std::make_shared<Room>(message);
        rooms.push_back(room);
    }
    addUser(user, rooms[index], clientSocket);
    return user;
}

void Server::clientMessaging(SOCKET clientSocket)
{
}

Server::~Server() {
    closesocket(serverSocket);
    WSACleanup();
}

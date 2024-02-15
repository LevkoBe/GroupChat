#include "Server.h"

// public
Server::Server(std::mutex& consoleMutex) : consoleMutex(consoleMutex), messenger(consoleMutex) {
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

void Server::handleClient(SOCKET clientSocket) {
    clients.push_back(clientSocket);

    std::shared_ptr<User> user = registerClient(clientSocket); // put the user into a group
    {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "Client " << clientSocket << " added to a group.\n";
    }
    receiveMessages(user);
}

Server::~Server() {
    closesocket(serverSocket);
    WSACleanup();
}


// helpers
std::string Server::invitingMessage(const std::string& username) {

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
    message += "If you have list of existing groups, you can join to any of them by just entering their INDEX.\n";
    message += "If you are going to create new group, simply type its NAME, and hit 'Enter'.\n";

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

std::shared_ptr<Room> Server::roomByString(std::string& roomString) {
    for (auto& room : rooms) {
        if (room->groupName == roomString) {
            return room;
        }
    }
    return nullptr;
}


// messaging
void Server::receiveMessages(std::shared_ptr<User> user) {
    std::string message;
    std::shared_ptr<Room> room = roomByString(user->room);
    std::shared_ptr<Message> userMessage = std::make_shared<Message>();
    while (true) {
        std::string path;
        char method = Common::receiveOptionType(user->clientSocket);
        message = Common::receiveChunkedData(user->clientSocket);
        switch (method) {
        case 'm':
            *userMessage = Message(message, user->username, roomByString(user->room));
            messenger.addMessageToQueue(userMessage);
            break;
        case 'f':
            path = "serverFolder\\" + user->room;
            Common::removeFolderContents(path);
            Common::createFile(message, path);
            break;
        case 'a':
            path = "serverFolder\\" + user->room;
            Common::appendToFile(message, path);
            break;
        case 's':
            path = "serverFolder\\" + user->room;
            message = Common::getFirstFile(path);
            if (message != "") {
                *userMessage = Message(message, user->username, roomByString(user->room), FileRequest);
                messenger.addMessageToQueue(userMessage);
            }
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


// registration
std::shared_ptr<User> Server::registerClient(SOCKET clientSocket) {
    int index;

    std::string username = askForUsername(clientSocket);
    std::shared_ptr<User> user = std::make_shared<User>(username, clientSocket);

    std::string groupname = askForGroupname(clientSocket, username);

    if (!tryParseInt(groupname, index) || index-- > rooms.size() || index < 0) {
        index = rooms.size();
        std::shared_ptr<Room> room = std::make_shared<Room>(groupname, user);
        rooms.push_back(room);
        room->password = askForPassword(clientSocket, -1);
        addUser(user, rooms[index], clientSocket);
        return user;
    }

    std::string password = askForPassword(clientSocket, index);
    if (password == rooms[index]->password)
        addUser(user, rooms[index], clientSocket);
    else Common::sendChunkedData(clientSocket, '-', "Incorrect password.");
    return user;
}

void Server::addUser(std::shared_ptr<User> user, std::shared_ptr<Room> room, SOCKET clientSocket) {
    room->users.push_back(user);
    user->room = room->groupName;
    Common::sendChunkedData(user->clientSocket, 'm', ("You've been added successfully to the group '" + user->room + "'.\n"));
    for (auto& message : room->messageHistory) {
        Common::sendChunkedData(clientSocket, 'm', message);
    }
    Common::sendChunkedData(clientSocket, 'h', "    You joined this group   ");
}

std::string Server::askForPassword(SOCKET clientSocket, int index) {
    std::string message;
    if (index == -1) {
        message = "If you want, you can create a PASSWORD for your group.\n";
        message += "Otherwise, enter '-', and hit Enter.\n";
        Common::sendChunkedData(clientSocket, 'm', message);
        Common::receiveOptionType(clientSocket);
        return Common::receiveChunkedData(clientSocket);
    }
    message = "Please, enter the password to the group.";

    if (rooms[index]->password == "-") {
        Common::sendChunkedData(clientSocket, '-', "-");
        return "-";
    }
    Common::sendChunkedData(clientSocket, 'm', "Please, enter the password: ");
    Common::receiveOptionType(clientSocket);
    return Common::receiveChunkedData(clientSocket);
}

std::string Server::askForUsername(SOCKET clientSocket) {
    Common::sendChunkedData(clientSocket, 'm', "Please, enter your name to connect to the server");
    Common::receiveOptionType(clientSocket);
    return Common::receiveChunkedData(clientSocket);
}

std::string Server::askForGroupname(SOCKET clientSocket, const std::string& username) {

    std::string message = invitingMessage(username);
    Common::sendChunkedData(clientSocket, 'm', message);
    Common::receiveOptionType(clientSocket);
    return Common::receiveChunkedData(clientSocket);
}

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

    std::string username = askForUsername(clientSocket);
    std::shared_ptr<User> user = std::make_shared<User>(username, clientSocket);

    while (user->state != Disconnected) { // otherwise -- disconnect

        std::shared_ptr<Room> room = joinRoom(user);
        {
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "Client " << clientSocket << " added to a group.\n";
        }
        receiveMessages(user, room);
    }
    clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
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
        if (rooms[i]->password != "-") {
            message += "[" + std::to_string(i + 1) + ". " + rooms[i]->groupName + " (" + std::to_string(rooms[i]->users.size()) + ")]\n";
        }
        else {
            message += std::to_string(i + 1) + ". " + rooms[i]->groupName + " (" + std::to_string(rooms[i]->users.size()) + ")\n";
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
void Server::receiveMessages(std::shared_ptr<User> user, std::shared_ptr<Room> room) {
    std::string message;
    std::string path = "serverFolder\\" + user->room;
    std::shared_ptr<Message> userMessage = std::make_shared<Message>();
    while (user->state == InRoom) {
        char method = receiveSignal(user);
        message = Common::receiveChunkedData(user->clientSocket);
        switch (method) {
        case 'm':
            *userMessage = Message(message, user->username, user->clientSocket, roomByString(user->room));
            messenger.addMessageToQueue(userMessage);
            break;
        case 'f':
            Common::removeFolderContents(path); // server limitations:
            Common::createFile(message, path); // storing only the last sent file
            break;
        case 'a':
            Common::appendToFile(message, path);
            break;
        case 'r':
            message = Common::getFirstFile(path);
            if (message != "") {
                *userMessage = Message(message, user->username, user->clientSocket, room, FileRequest);
                messenger.addMessageToQueue(userMessage);
            }
            break;
        case 's':
            userMessage = std::make_shared<Message>(message, user->username, user->clientSocket, room, File);
            messenger.addMessageToQueue(userMessage);
            break;
        case 'x':
            message = "     User " + user->username + " left the group.     ";
            *userMessage = Message(message, user->username, user->clientSocket, room);
            messenger.addMessageToQueue(userMessage);

            Common::sendChunkedData(user->clientSocket, 'x', "You can join new room!");
            user->state = Connected;
            break;
        case '-':
        default:
            message = "     User " + user->username + " left the group.     ";
            *userMessage = Message(message, user->username, user->clientSocket, room);
            messenger.addMessageToQueue(userMessage);

            Common::sendChunkedData(user->clientSocket, '-', "Thank you for being with us!", 100);
            std::lock_guard<std::mutex> lock(consoleMutex);
            std::cout << "Client " << user->clientSocket << " disconnected.\n";
            closesocket(user->clientSocket);
            break;
        }
    }
}

char Server::receiveSignal(std::shared_ptr<User> user) {
    char signal = Common::receiveOptionType(user->clientSocket);
    switch (signal) {
        case '-':
            user->state = Disconnected;
            break;
        case 'x':
            user->state = Connected;
            break;
        default:
            break;
    }
    return signal;
}


// registration

std::shared_ptr<Room> Server::joinRoom(std::shared_ptr<User> user) {
    std::shared_ptr<Room> room;
    int index;

    while (user->state == Connected) {
        std::string groupname = askForGroupname(user->clientSocket, user->username);

        if (!tryParseInt(groupname, index) || index-- > rooms.size() || index < 0) {
            user->room = groupname;
            user->state = InRoom;
            return newRoom(user);
        }

        {
            std::scoped_lock<std::mutex> lock(roomsLock);
            room = rooms[index]; // retrieve room
        }
        if (askForPassword(user, room->password) && user->state == Connected) {
            addUser(user, room, user->clientSocket);
            return room;
        }
    }
}
std::shared_ptr<Room> Server::newRoom(std::shared_ptr<User> user) {
    std::shared_ptr<Room> room = std::make_shared<Room>(user->room, user); // new room

    std::string message = "If you want, you can create a PASSWORD for your group.\n";
    message += "Otherwise, enter '-', and hit Enter.\n";
    Common::sendChunkedData(user->clientSocket, '?', message);
    Common::receiveOptionType(user->clientSocket);
    room->password = Common::receiveChunkedData(user->clientSocket); // new password

    std::scoped_lock<std::mutex> lock(roomsLock);
    rooms.push_back(room);
    addUser(user, room, user->clientSocket);
    return room;
}
void Server::addUser(std::shared_ptr<User> user, std::shared_ptr<Room> room, SOCKET clientSocket) {
    room->users.push_back(user);
    user->room = room->groupName;
    user->state = InRoom;

    Common::sendChunkedData(user->clientSocket, 'm', ("You've been added successfully to the group '" + user->room + "'.\n"));
    for (auto& message : room->messageHistory) {
        Common::sendChunkedData(clientSocket, 'm', message);
    }
    Common::sendChunkedData(clientSocket, 'h', "    You joined this group   ");

    std::string message = "    " + user->username + " joined the group    ";
    std::shared_ptr<Message> userMessage = std::make_shared<Message>(message, user->username, user->clientSocket, room, Text);
    messenger.addMessageToQueue(userMessage);
}

bool Server::askForPassword(std::shared_ptr<User> user, const std::string& password) {

    if (password == "-") {
        Common::sendChunkedData(user->clientSocket, 'm', "No password required");
        return true;
    }

    std::string message = "Please, enter the password to the group.";
    Common::sendChunkedData(user->clientSocket, '?', "Please, enter the password: ");
    receiveSignal(user);
    if (password == Common::receiveChunkedData(user->clientSocket)) return true;
    
    Common::sendChunkedData(user->clientSocket, 'm', "Incorrect password.");
    return false;
}
std::string Server::askForUsername(SOCKET clientSocket) {
    Common::sendChunkedData(clientSocket, '?', "Please, enter your name to connect to the server");
    Common::receiveOptionType(clientSocket);
    return Common::receiveChunkedData(clientSocket);
}
std::string Server::askForGroupname(SOCKET clientSocket, const std::string& username) {

    std::string message = invitingMessage(username);
    Common::sendChunkedData(clientSocket, '?', message);
    Common::receiveOptionType(clientSocket);
    return Common::receiveChunkedData(clientSocket);
}

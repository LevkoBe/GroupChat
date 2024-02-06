#include "Client.h"


Client::Client() { // todo
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(8080);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        closesocket(clientSocket);
        WSACleanup();
        return;
    }
}

bool Client::sendMessage(const char operationType, const std::string& message) {
    return Common::sendChunkedData(clientSocket, operationType, message, 100);
}

std::string Client::receiveMessage() {
    return Common::receiveChunkedData(clientSocket);
}

void Client::receiveMessages(SOCKET clientSocket) {
    std::string message;
    while (true) {
        char option = Common::receiveOptionType(clientSocket);
        switch (option) {
        case 'm':
            message = Common::receiveChunkedData(clientSocket);
            std::cout << "Server: " << message << std::endl;
            break;
        case 'f':
            message = Common::receiveChunkedData(clientSocket);
            std::cout << "Server: " << message << std::endl;
            break;
        default:
            std::cerr << "Server disconnected.\n";
            return;
            break; ///
        }
    }
}

Client::~Client() {
    closesocket(clientSocket);
    WSACleanup();
}

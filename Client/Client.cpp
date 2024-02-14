#include "Client.h"
#include <mutex>

std::mutex consoleMutex;

// basic
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
Client::~Client() {
    closesocket(clientSocket);
    WSACleanup();
}

// messaging
bool Client::sendMessage(const char operationType, const std::string& message) {
    bool result = Common::sendChunkedData(clientSocket, operationType, message, 100);
    return result;
}
void Client::receiveMessages() {
    std::string message;
    while (true) {
        char option = Common::receiveOptionType(clientSocket);
        message = Common::receiveChunkedData(clientSocket);
        switch (option) {
        case 'm':
            print(message, 2);
            break;
        case 'f':
            print(message, 2);
            break;
        default:
            std::cerr << "Server disconnected.\n";
            return;
            break; ///
        }
    }
}

std::string Client::receiveMessage() {
    Common::receiveOptionType(clientSocket);
    std::string message = Common::receiveChunkedData(clientSocket);
    return message;
}

void Client::receiveHistory() {
    char option;
    std::string message;
    do {
        option = Common::receiveOptionType(clientSocket);
        message = Common::receiveChunkedData(clientSocket);
        std::cout << message << std::endl;
    } while (option != 'h');
}

// files
void Client::saveFile() {

}

void Client::sendFile() {

}

// console
void Client::clearLastLine() {
    std::cout << "\x1b[1A\x1b[2K";
}
void Client::print(const std::string& output, int numLines) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    for (int i = 0; i < numLines; i++) clearLastLine();

    if (output != "") std::cout << output << std::endl;
    std::cout << "------------------------------------------------------------------------------------------\n";
    std::cout << "Type : save(to download a file), file(to send a file) or just a message.Then hit ENTER.\n";
    std::cout << "SEND : ";
}


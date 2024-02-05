// Client.cpp
#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

class Client {
    WSADATA wsaData;
    sockaddr_in serverAddr;
public:
    SOCKET clientSocket;

    Client() {
        //WSADATA wsaData;
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

        //sockaddr_in serverAddr;
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

    ~Client() {
        closesocket(clientSocket);
        WSACleanup();
    }
};

void receiveMessages(SOCKET clientSocket) {
    char buffer[4096];
    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Server disconnected.\n";
            break;
        }

        buffer[bytesReceived] = '\0';
        std::cout << "Server: " << buffer << std::endl;
    }
}

int main() {
    Client client;
    SOCKET clientSocket = client.clientSocket;
    std::cout << "Connected to server.\n";


    std::thread receiveThread(receiveMessages, clientSocket);

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        send(clientSocket, message.c_str(), message.size() + 1, 0);
    }
    receiveThread.join();

    return 0;
}

//cd C:\Users\1levk\source\repos\GroupChat\x64\Debug
 //Client.exe
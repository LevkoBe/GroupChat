#include "Client.h"
#include "Common.h"
#include <thread>


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
        client.sendMessage('m', message);
    }
    receiveThread.join();

    return 0;
}

//cd C:\Users\1levk\source\repos\GroupChat\x64\Debug
 //Client.exe
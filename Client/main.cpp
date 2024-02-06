#include "Client.h"
#include "Common.h"
#include <thread>

Client client;

void receiveMessages(SOCKET clientSocket) {
    client.receiveMessages(clientSocket);
}

int main() {
    SOCKET clientSocket = client.clientSocket;
    std::cout << "Connected to server.\n";

    std::thread receiveThread(receiveMessages, clientSocket);

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "   " || message == "stop" || !client.sendMessage('m', message)) {
            client.sendMessage('-', message);
            std::cout << client.receiveMessage();
            break;
        }
    }

    receiveThread.join();
    return 0;
}

//cd C:\Users\1levk\source\repos\GroupChat\x64\Debug
 //Client.exe
#include "Client.h"
#include "Common.h"
#include <thread>

Client client;

void receiveMessages() {
    client.receiveMessages();
}

int main() {
    SOCKET clientSocket = client.clientSocket;
    std::cout << "Connected to server.\n";

    std::cout << "Authorization started...\n";
    std::string message;

    // reading and sending USERNAME, GROUPNAME, and PASSWORD
    for (int i = 0; i < 3; i++) {
        std::string received = client.receiveMessage();
        std::cout << received << std::endl; // server's invitation

        if (received != "-") { // if password required
            std::getline(std::cin, message);
            client.sendMessage('m', message);
        }
    }

    std::thread receiveThread(receiveMessages);

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
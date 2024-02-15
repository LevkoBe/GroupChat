#include "Client.h"
#include "Common.h"
#include <thread>

Client client;

void receiveMessages() {
    client.receiveMessages();
}

void authorization() {
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

    system("cls");
}

int main() {
    SOCKET clientSocket = client.clientSocket;
    std::cout << "Connected to server.\n";

    authorization();
    std::cout << client.receiveMessage() << std::endl; // group admission

    client.receiveHistory();

    std::thread receiveThread(receiveMessages);

    std::string message;
    client.print("", 0);
    while (true) {
        int numLines = 2;
        do {
            std::getline(std::cin, message);
            numLines++;
        } while (message == "");


        if (message == "_file") {
            if (client.sendFile()) {
                client.print("File was sent.", 3, 4);
            } else {
                client.print("File wasn't sent.", 3, 5);
            }
        }
        else if (message == "_save") {
            client.sendMessage('s', "save");
            client.print("", 4);
        }
        else if (message == "   " || message == "stop" || !client.sendMessage('m', message)) { // send
            client.sendMessage('-', message);
            std::cout << client.receiveMessage();
            break;
        } else {
            client.print("You: " + message, numLines, 1); // print
        }
    }

    receiveThread.join();
    return 0;
}

//cd C:\Users\1levk\source\repos\GroupChat\x64\Debug
//Client.exe
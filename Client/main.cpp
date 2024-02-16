#include "Client.h"
#include <thread>

Client client;

void receiveMessages(std::mutex& threadExclusor, std::condition_variable& inGroup) {
    client.receiveMessages(threadExclusor, inGroup);
}

int main() {
    std::mutex threadExclusor;
    std::condition_variable inGroup;

    SOCKET clientSocket = client.clientSocket;
    std::cout << "Connected to server.\n";
    bool connected = true;
    bool inGroup = false;

    client.introduceYourself();

    std::thread receiveThread(receiveMessages, inGroup);
    while (connected) {
        {
            std::scoped_lock<std::mutex> lock(threadExclusor);
            client.joinGroup();
        }

        connected = client.sendMessages();
    }


    return 0;
}

//cd C:\Users\1levk\source\repos\GroupChat\x64\Debug
//Client.exe
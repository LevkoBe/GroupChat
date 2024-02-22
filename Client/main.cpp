#include "Client.h"
#include "Common.h"
#include "Globals.h"
#include <thread>

Client client;

void receiveMessages(std::atomic<State>& state) {
    client.receiveMessages(state);
}


int main() {
    SOCKET clientSocket = client.clientSocket;
    std::cout << "Connected to server.\n";
    std::atomic<State> state = Entering;

    Common::receiveOptionType(clientSocket);
    std::cout << Common::receiveChunkedData(clientSocket) << std::endl;
    std::string username;
    std::getline(std::cin, username);
    Common::sendChunkedData(clientSocket, '?', username);

    client.enterGroup(state);

    std::thread receiveThread(receiveMessages, std::ref(state));
    while (state != Disconnected) {
        if (state == Waiting)
            continue;
        else if (state == Entering)
            client.enterGroup(state);
        else if (!client.sendMessage(state))
            state = Disconnected;
    }

    receiveThread.join();
    return 0;
}

//cd C:\Users\1levk\source\repos\GroupChat\x64\Debug
//Client.exe
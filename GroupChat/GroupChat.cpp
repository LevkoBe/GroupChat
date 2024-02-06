#include "Server.h"

Server server;

#pragma comment(lib, "ws2_32.lib")
std::mutex consoleMutex;
std::vector<SOCKET> clients;

void handleClient(SOCKET clientSocket) {
    server.handleClient(clientSocket, consoleMutex);
}

int main() {

    while (server.serverSocket) { // ?
        SOCKET clientSocket = accept(server.serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            Common::errorMessage("Accept failed.\n");
            closesocket(server.serverSocket);
            WSACleanup();
            return 1;
        }

        std::lock_guard<std::mutex> lock(consoleMutex); // ?
        std::cout << "Client " << clientSocket << " connected.\n";

        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach(); // Detach the thread to allow handling multiple clients concurrently // ???
    }

    return 0;
}

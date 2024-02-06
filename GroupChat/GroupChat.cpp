#include "Server.h"
#pragma comment(lib, "ws2_32.lib")

Server server;
std::mutex consoleMutex;
std::vector<std::thread> clients;

void handleClient(SOCKET clientSocket) {
    server.handleClient(clientSocket, consoleMutex);
}

int main() {

    while (server.serverSocket) {
        SOCKET clientSocket = accept(server.serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            Common::errorMessage("Accept failed.\n");
            closesocket(server.serverSocket);
            WSACleanup();
            break;
        }

        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout << "Client " << clientSocket << " connected.\n";

        clients.emplace_back(handleClient, clientSocket);
        //std::thread clientThread(handleClient, clientSocket);
        //clientThread.detach(); // ?
    }
    for (auto& client : clients) {
        client.join();
    }

    return 0;
}

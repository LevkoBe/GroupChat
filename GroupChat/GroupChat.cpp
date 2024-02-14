#include "Server.h"
#pragma comment(lib, "ws2_32.lib")

std::mutex consoleMutex;
Server server(consoleMutex);
std::vector<std::thread> clients;

void handleClient(SOCKET clientSocket) {
    server.handleClient(clientSocket);
}

int main() {

    while (server.serverSocket) { // connecting new clients
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
    }
    for (auto& client : clients) {
        client.join();
    }

    return 0;
}

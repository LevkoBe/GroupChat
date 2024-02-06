#include "Common.h"
#include <mutex>

std::mutex m;

bool Common::errorMessage(const std::string& message) {
    m.lock();
    std::cerr << message << std::endl;
    m.unlock();
    return false;
}

bool Common::sendChunkedData(SOCKET clientSocket, const char operationType, const std::string& messageStr, int chunkSize) {

    const char* message = messageStr.c_str();
    int dataSize = strlen(message);
    if (send(clientSocket, reinterpret_cast<const char*>(&operationType), sizeof(char), 0) == SOCKET_ERROR) {
        return errorMessage("Failed to send type of the operation.");
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&chunkSize), sizeof(int), 0) == SOCKET_ERROR) {
        return errorMessage("Failed to send chunk size.");
    }
    if (send(clientSocket, reinterpret_cast<const char*>(&dataSize), sizeof(int), 0) == SOCKET_ERROR) {
        return errorMessage("Failed to send total size.");
    }

    int totalSent = 0;

    while (totalSent < dataSize)
    {
        int remaining = dataSize - totalSent;
        int currentChunkSize = (remaining < chunkSize) ? remaining : chunkSize;

        if (send(clientSocket, message + totalSent, currentChunkSize, 0) == SOCKET_ERROR) {
            return errorMessage("Failed to send chunked data.");
        }
        totalSent += currentChunkSize;
    }
    return true;

}

std::string Common::receiveChunkedData(SOCKET clientSocket)
{
    int chunkSize;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&chunkSize), sizeof(int), 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
        errorMessage("Error in receiving chunk size.");
        return "";
    }
    int totalSize;
    bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&totalSize), sizeof(int), 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
        errorMessage("Error in receiving total size.");
        return "";
    }

    // Receive message
    std::string assembledData;
    int totalReceived = 0;

    char* buffer = new char[chunkSize + 1];
    while (totalReceived < totalSize)
    {
        int remaining = totalSize - totalReceived;
        int currentChunkSize = (remaining < chunkSize) ? remaining : chunkSize;
        int bytesReceived = recv(clientSocket, buffer, currentChunkSize, 0);
        buffer[bytesReceived] = '\0';

        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            errorMessage("Error in receiving chunked data.");
            return "";
        }

        assembledData.append(buffer, bytesReceived);
        totalReceived += bytesReceived;
    }

    delete[] buffer;
    return assembledData;
}

char Common::receiveOptionType(SOCKET clientSocket) {
    char option;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&option), sizeof(char), 0);
    if (bytesReceived == SOCKET_ERROR) {
        return '-';
    }
    return option;
}

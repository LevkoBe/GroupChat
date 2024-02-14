#include "Common.h"

std::mutex m;

bool Common::errorMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(m);
    std::cerr << message << std::endl;
    return false;
}

// basic send-receive mehthods
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

// file handling methods
bool Common::sendFile(SOCKET clientSocket, const std::string& filepath) {
    const int chunkSize = 1024 * 1024;
    char* buffer = new char[chunkSize + 1];

    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open()) {
        std::string message = "Error: unable to open file.";
        sendChunkedData(clientSocket, 'm', message);
        delete[] buffer;
        return false;
    }

    while (!file.eof()) {
        file.read(buffer, chunkSize);
        std::streamsize bytesRead = file.gcount();
        buffer[bytesRead] = '\0';

        if (bytesRead > 0) {
            char indicator = (!file.eof()) ? '+' : '-';
            std::cout << "chunk " << indicator << std::endl;
            std::string message = buffer;
            sendChunkedData(clientSocket, indicator, message);
        }
    }
    delete[] buffer;

    file.close();
    return true;
}

bool Common::createFile(const std::string& filepath) {
    std::vector<std::string> file = splitStringInTwo(filepath);
    std::string message;

    if (file.size() != 2) {
        std::cerr << "Sorry, the message received is: '" + filepath + "', but expected were name and content of a file.";
        return false;
    }

    std::string filename = file[0];
    std::string fileContent = file[1];

    std::ofstream outputFile(filename);
    if (!outputFile.is_open()) {
        std::cerr << "Sorry, the file is already in use.";
        return false;
    }

    outputFile << fileContent;
    outputFile.close();

    std::cerr << "File succesfully written!";
    return true;
}

bool Common::appendToFile(const std::string& filepath) {
    std::vector<std::string> file = splitStringInTwo(filepath);
    std::string message;

    if (file.size() != 2) {
        std::cerr << "Sorry, not received: 1. file name, 2. file content;\n";
        return false;
    }

    std::string filename = file[0];
    std::string fileContent = file[1];
    std::ofstream outputFile(filename, std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "Sorry, the file is already in use.";
        return false;
    }

    outputFile << fileContent;
    outputFile.close();

    message = "File succesfully written!";
    return true;
}

bool Common::remove(const std::string& filepath)
{
    try {
        fs::remove_all(filepath);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting file/folder: " << e.what() << std::endl;
        return false;
    }
}


// helpers
std::vector<std::string> Common::splitStringInTwo(const std::string& str, char delimiter) {
    std::string part;
    std::vector<std::string> parts;
    std::istringstream stream(str);

    std::getline(stream, part, delimiter); // read the filename
    parts.push_back(part);
    std::getline(stream, part, '\0'); // read all the rest
    parts.push_back(part);
    return parts;
}

std::string Common::fullPath(const std::string& filename, const std::string& username) {
    if (username == "adminPassword") {
        return (fs::path("serverFolder/") / filename).string();
    }
    return (fs::path("serverFolder/" + username + "/") / filename).string();
}

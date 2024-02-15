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
bool Common::sendFile(SOCKET clientSocket, const std::string& filename, const std::string& pathAdd) {
    const int chunkSize = 1024 * 1024;
    char* buffer = new char[chunkSize + 1];
    int prefixLength = filename.length() + 1;

    fs::path filepath = pathAdd == "" ? fs::current_path() / filename :
        fs::current_path() / pathAdd / filename;
    std::ifstream file(filepath, std::ios::binary);

    if (!file.is_open()) {
        std::string message = "Error: unable to open file.";
        sendChunkedData(clientSocket, 'm', message);
        delete[] buffer;
        return false;
    }

    char indicator = 'f';
    while (!file.eof()) {
        file.read(buffer, chunkSize - prefixLength);
        std::streamsize bytesRead = file.gcount();
        buffer[bytesRead] = '\0';

        if (bytesRead > 0) {
            std::cout << "chunk " << indicator << std::endl;
            std::string message = filename + '\n' + buffer;
            sendChunkedData(clientSocket, indicator, message);
            indicator = 'a';
        }
    }
    delete[] buffer;

    file.close();
    return true;
}

bool Common::createFile(std::string& content, const std::string& pathAdd) {

    std::vector<std::string> file = splitStringInTwo(content);
    std::string message;

    if (file.size() != 2) {
        std::cerr << "Sorry, expected were name and content of a file.";
        return false;
    }

    fs::path filepath = pathAdd == "" ? fs::current_path() / file[0] :
        fs::current_path() / pathAdd / file[0];
    std::string fileContent = file[1];

    std::ofstream outputFile(filepath);
    if (!outputFile.is_open()) {
        std::cerr << "Sorry, the file " << filepath << " is already in use.\n";
        return false;
    }

    outputFile << fileContent;
    outputFile.close();

    return true;
}

bool Common::appendToFile(std::string& content, const std::string& pathAdd) {
    std::vector<std::string> file = splitStringInTwo(content);
    std::string message;

    if (file.size() != 2) {
        std::cerr << "Sorry, not received: 1. file name, 2. file content;\n";
        return false;
    }

    fs::path filepath = pathAdd == "" ? fs::current_path() / file[0] :
        fs::current_path() / pathAdd / file[0];
    std::string fileContent = file[1];
    std::ofstream outputFile(filepath, std::ios::app);
    if (!outputFile.is_open()) {
        std::cerr << "Sorry, the file is already in use.";
        return false;
    }

    outputFile << fileContent;
    outputFile.close();

    message = "File succesfully written!";
    return true;
}

bool Common::removeFolderContents(const std::string& folderPath) {
    fs::path resultingPath = fs::current_path() / folderPath;
    try {
        for (const auto& entry : fs::directory_iterator(resultingPath)) {
            if (fs::is_regular_file(entry)) {
                fs::remove(entry.path());
            }
            else if (fs::is_directory(entry)) {
                fs::remove_all(entry.path());
            }
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error deleting folder contents: " << e.what() << std::endl;
        return false;
    }
}

std::string Common::getFirstFile(const std::string& folderPath) {
    fs::path resultingPath = fs::current_path() / folderPath;
    for (const auto& entry : fs::directory_iterator(resultingPath)) {
        if (fs::is_regular_file(entry)) {
            return entry.path().filename().string();
        }
    }
    return "";
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


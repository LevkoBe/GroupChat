#include "Client.h"
#include <mutex>

std::mutex consoleMutex;

// basic
Client::Client() { // todo
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(8080);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        closesocket(clientSocket);
        WSACleanup();
        return;
    }
}
Client::~Client() {
    closesocket(clientSocket);
    WSACleanup();
}

bool Client::introduceYourself() {

    std::string message;

    std::string received = receiveMessage();
    std::cout << received << std::endl; // server's invitation

    std::getline(std::cin, message);
    sendMessage('m', message);

    return true;
}

bool Client::joinGroup() {
    std::string message;

    std::cout << receiveMessage();
    std::getline(std::cin, message); // groupname
    sendMessage('m', message);

    std::string received = receiveMessage();
    std::cout << received << std::endl; // password request

    if (received != "-") { // if password required
        std::getline(std::cin, message);
        sendMessage('m', message);
    }

    system("cls");
    std::cout << receiveMessage() << std::endl; // group admission
    receiveHistory();
    return true;
}

// messaging

bool Client::sendMessages() {
    std::string message;
    print("", 0);

    while (true) {
        int numLines = 2;
        do {
            std::getline(std::cin, message);
            numLines++;
        } while (message == "");


        if (message == "_file") {
            if (sendFile()) {
                print("File was sent.", 5, 4);
            } else {
                print("File wasn't sent.", 5, 5);
            }
        } else if (message == "_save") {
            sendMessage('s', "save");
            print("", 4);
        } else if (message == "_rejoin") {
            system("cls");
            sendMessage('r', "rejoin");
            return true;
            break;
        } else if (message == "   " || message == "stop" || !sendMessage('m', message)) { // send
            sendMessage('-', message);
            std::cout << receiveMessage();
            return false;
            break;
        } else {
            print("You: " + message, numLines, 1); // print
        }
    }

}

bool Client::sendMessage(const char operationType, const std::string& message) {
    bool result = Common::sendChunkedData(clientSocket, operationType, message, 100);
    return result;
}
void Client::receiveMessages(std::mutex& threadExclusor, std::condition_variable& inGroup) {
    std::string message;
    bool working = true;
    while (working) {
        std::unique_lock<std::mutex> lock(dataMutex);
        dataCondition.wait(lock, [ ] { return sharedData != 0; });
    }
    while (working) {
        char option = Common::receiveOptionType(clientSocket);
        message = Common::receiveChunkedData(clientSocket);
        switch (option) {
        case 'm':
            print(message, 2, 3);
            break;
        case 'f':
            Common::createFile(message);
            print("file downloaded.", 2, 4);
            break;
        case 'a':
            Common::appendToFile(message);
            break;
        case 'r':
            working = false;
            break;
        default:
            std::cerr << "Server disconnected.\n";
            return;
            break; ///
        }
    }
}

std::string Client::receiveMessage() {
    Common::receiveOptionType(clientSocket);
    std::string message = Common::receiveChunkedData(clientSocket);
    return message;
}

void Client::receiveHistory() {
    char option;
    std::string message;
    do {
        option = Common::receiveOptionType(clientSocket);
        message = Common::receiveChunkedData(clientSocket);
        std::cout << message << std::endl;
    } while (option != 'h');
}

// files

bool Client::sendFile() {
    std::string filename;
    std::scoped_lock<std::mutex> lock(consoleMutex);
    std::cout << "Enter file name you'd like to send: ";
    std::getline(std::cin, filename);
    if (Common::sendFile(clientSocket, filename)) {
        sendMessage('m', "FILE");
        return true;
    }
    return false;
}

// console
void Client::clearLastLine() {
    std::cout << "\x1b[1A\x1b[2K";
}
void Client::setPalette(int palette) {
    switch (palette)
    {
    case 0:
        std::cout << "\033[0m";
        break;
    case 1:
        std::cout << "\033[0;33m";
        break;
    case 2:
        std::cout << "\033[1;32m";
        break;
    case 3:
        std::cout << "\033[1;33m";
        break;
    case 4:
        std::cout << "\033[0;32m";
        break;
    case 5:
        std::cout << "\033[0;31m";
        break;
    default:
        break;
    }
}
void Client::print(const std::string& output, int numLines, int palette) {
    std::lock_guard<std::mutex> lock(consoleMutex);
    for (int i = 0; i <= numLines; i++) clearLastLine();

    setPalette(palette);
    if (output != "") std::cout << output << std::endl;
    setPalette(0);
    std::cout << "------------------------------------------------------------------------\n";
    std::cout << "Type : _save (to download a file), _file (to send a file), \
                 \n_rejoin (to join different group) or just any message.Then hit ENTER.\n";
    std::cout << "SEND : ";
}


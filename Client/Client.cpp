#include "Client.h"
#include "Globals.h"

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

// Q&A
bool Client::answerQuestion(State& state) {
    //std::unique_lock<std::mutex> lock(recvLocker); /// systematic approach, but recv() will not be called anyway
    std::string questionAnswer = Common::receiveChunkedData(clientSocket);
    std::cout << questionAnswer << std::endl;
    char option = 'm';
    std::getline(std::cin, questionAnswer);
    if (questionAnswer == "save") {
        option = 'r'; // request to save the file
    }
    Common::sendChunkedData(clientSocket, option, questionAnswer);
    answerRequired.notify_one();
    state = Messaging;
    return true;
}

bool Client::enterGroup(State& state) {
    Common::receiveOptionType(clientSocket);
    std::cout << Common::receiveChunkedData(clientSocket) << std::endl;
    std::string groupName;
    std::getline(std::cin, groupName);
    if (!Common::sendChunkedData(clientSocket, '?', groupName)) return false;

    bool passwordRequired = Common::receiveOptionType(clientSocket) != 'm';
    std::cout << Common::receiveChunkedData(clientSocket) << std::endl;
    if (passwordRequired) {
        std::string password;
        std::getline(std::cin, password);
        if (!Common::sendChunkedData(clientSocket, '?', password)) return false;
    }

    state = Waiting;
    system("cls");
    answerRequired.notify_one();
    return true;
}

// messaging

bool Client::sendMessage() {
    int numLines = 2;
    std::string message;
    do {
        std::getline(std::cin, message);
        numLines++;
    } while (message == "");


    if (message == "_exit") {
        Common::sendChunkedData(clientSocket, 'x', "Rejoin");
    } else if (message == "_file") {
        if (sendFile()) {
            print("File was sent.", 5, 4);
        } else {
            print("File wasn't sent.", 5, 5);
        }
    } else if (message == "_save") {
        Common::sendChunkedData(clientSocket, 's', "save");
        print("", 4);
    } else if (message == "   " || message == "stop" || !Common::sendChunkedData(clientSocket, 'm', message)) { // send
        Common::sendChunkedData(clientSocket, '-', message);
        std::cout << receiveMessage();
        return false;
    }
    print("You: " + message, numLines, 1); // print
    return true;
}
void Client::receiveMessages(State& state) {
    char option;
    std::string message;
    while (state != Disconnected) {
        {
            std::unique_lock<std::mutex> lock(recvLocker);
            answerRequired.wait(lock, [ & ] { return (state == Messaging || state == Waiting); });
            option = Common::receiveOptionType(clientSocket);
            if (option == '?') {
                state = AnswerRequired;
                continue;
            }
            message = Common::receiveChunkedData(clientSocket);
        }
        switch (option) {
            case 'm':
                print(message, 2, 3);
                break;
            case 'f':
                Common::createFile(message);
                print("file is being downloaded...", 2, 4);
                break;
            case 'a':
                Common::appendToFile(message);
                break;
            case 's':
                print("file downloaded.", 2, 4);
                break;
            case 'h':
                state = Messaging;
                break;
            case 'x':
                state = Entering;
                break;
            case '-':
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
        Common::sendChunkedData(clientSocket, 'm', "FILE: " + filename);
        return true;
    }
    return false;
}

// console
void Client::clearLastLine() {
    std::cout << "\x1b[1A\x1b[2K";
}
void Client::setPalette(int palette) {
    switch (palette) {
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
    for (int i = 0; i < numLines; i++) clearLastLine();

    setPalette(palette);
    if (output != "") std::cout << output << std::endl;
    setPalette(0);
    std::cout << "------------------------------------------------------------------------------------------\n";
    std::cout << "Type : _save (to download a file), _file (to send a file) or just some message.Then hit ENTER.\n";
    std::cout << "SEND : ";
}


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

bool Client::enterGroup(std::atomic<State>& state) {
    char mark;
    std::string question;
    do {
        mark = Common::receiveOptionType(clientSocket);
        question = Common::receiveChunkedData(clientSocket);
    } while (mark != '?');
    std::cout << question << std::endl;
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

bool Client::sendMessage(std::atomic<State>& state) {
    int numLines = 2;
    std::string message;
    do {
        std::getline(std::cin, message);
        numLines++;
    } while (message == "");

    if (state == AnswerRequired) {
        if (std::tolower(message[0]) == 'y') {
            Common::sendChunkedData(clientSocket, 'r', message);
            state = Messaging;
            print("responded 'SAVE'", 5, 4);
        } else if (std::tolower(message[0] == 'n')) {
            state = Messaging;
            print("file wasn't saved", 5, 4);
        } else {
            print("Please, provide response 'Y'/'N': ", 4, 4); // todo: remove block of ---...
        }
        return true;
    }

    if (message == "_exit") {
        state = Waiting;
        Common::sendChunkedData(clientSocket, 'x', "Rejoin");
        return true;
    } else if (message == "_file") {
        if (sendFile()) {
            print("File was sent.", 4, 4);
        } else {
            print("File wasn't sent.", 3, 5);
        }
        return true;
    } else if (message == "_save") {
        Common::sendChunkedData(clientSocket, 'r', "save");
        print("Request to download file was sent.", numLines, 3);
        return true;
    } else if (message == "stop" || !Common::sendChunkedData(clientSocket, 'm', message)) { // send
        Common::sendChunkedData(clientSocket, '-', message);
        state = Disconnected;
        return false;
    }
    print("You: " + message, numLines, 1); // print
    return true;
}
void Client::receiveMessages(std::atomic<State>& state) {
    char option;
    std::string message;
    while (state != Disconnected) {
        {
            std::unique_lock<std::mutex> lock(recvLocker);
            answerRequired.wait(lock, [ & ] { return (state != Entering); });
            option = Common::receiveOptionType(clientSocket);
            message = Common::receiveChunkedData(clientSocket);
        }
        switch (option) {
            case '?':
                print(message, 2, 4);
                state = AnswerRequired;
                break;
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
                print("file downloaded.", 3, 4);
                break;
            case 'h':
                state = Messaging;
                break;
            case 'x':
                state = Entering;
                break;
            case '-':
            default:
                state = Disconnected;
                std::cerr << "Server disconnected.\n";
                return;
                break;
        }
    }
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


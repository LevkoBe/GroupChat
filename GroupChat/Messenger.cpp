#include "Messenger.h"

std::queue<std::shared_ptr<Message>> messageQueue;

//std::mutex m;

void Messenger::broadcastMessage(const Message& message, std::mutex& consoleMutex) {

    std::shared_ptr<Room> room = message.room;
    std::string folderpath;

    {
        std::scoped_lock<std::mutex> lock(room->roomLock); // to avoid accessing removed users

        switch (message.type) {
            case Text:
                room->messageHistory.push_back(message.toStr());
                for (std::shared_ptr<User> user : room->users) {
                    if (user->clientSocket != message.senderSocket) {
                        Common::sendChunkedData(user->clientSocket, 'm', message.toStr());
                    }
                }
                break;
            case File:
                room->messageHistory.push_back(message.toStr());
                for (std::shared_ptr<User> user : room->users) {
                    if (user->clientSocket != message.senderSocket) {
                        Common::sendChunkedData(user->clientSocket, 'm', message.toStr());
                        Common::sendChunkedData(user->clientSocket, '?', "Do you want to download the file (Y/N)?");
                    }
                }
                break;
            case FileRequest:
                for (std::shared_ptr<User> user : room->users) {
                    if (user->clientSocket == message.senderSocket) {
                        folderpath = "serverFolder\\" + room->groupName;
                        Common::sendFile(user->clientSocket, message.message, folderpath);
                    }
                }
                break;
            default:
                break;
        }
    }
    std::scoped_lock<std::mutex> lock(consoleMutex);
    std::cout << "Client " << message.sender << ": " << message << std::endl;
}

void Messenger::broadcastMessages(std::mutex& consoleMutex) {
    while (true) {
        std::unique_lock<std::mutex> lock(messageQueueMutex);
        messageAvailableCondition.wait(lock, [&](){ return !messageQueue.empty(); });

        while (!messageQueue.empty()) {
            std::shared_ptr<Message> message = messageQueue.front();
            messageQueue.pop();
            broadcastMessage(*message, consoleMutex);
        }
    }
}


void Messenger::addMessageToQueue(std::shared_ptr<Message> message) {
    {
        std::scoped_lock<std::mutex> lock(messageQueueMutex);
        messageQueue.push(message);
    }
    messageAvailableCondition.notify_one();
}

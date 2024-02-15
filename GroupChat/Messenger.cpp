#include "Messenger.h"

std::queue<std::shared_ptr<Message>> messageQueue;

void Messenger::broadcastMessage(const Message& message, std::mutex& consoleMutex) {

    std::shared_ptr<Room> room = message.room;
    std::string folderpath;
    std::string filename;
    switch (message.type) {
    case Text:
        room->messageHistory.push_back(message.toStr());
        for (std::shared_ptr<User> user : room->users) {
            if (user->username != message.sender) {
                Common::sendChunkedData(user->clientSocket, 'm', message.toStr(), 100);
            }
        }
        break;
    case FileRequest:
        for (std::shared_ptr<User> user : room->users) {
            if (user->username == message.sender) { // todo: remake into sockets
                folderpath = "serverFolder\\" + room->groupName;
                filename = Common::getFirstFile(folderpath);
                Common::sendFile(user->clientSocket, filename, folderpath);
            }
        }
        break;
    default:
        break;
    }
    std::lock_guard<std::mutex> lock(consoleMutex);
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

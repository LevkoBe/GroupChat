#include "Messenger.h"

std::queue<std::shared_ptr<Message>> messageQueue;

void Messenger::broadcastMessage(const Message& message, std::mutex& consoleMutex) {

    std::shared_ptr<Room> room = message.room;
    room->messageHistory.push_back(message.toStr());
    for (std::shared_ptr<User> user : room->users) {
        if (user->username != message.sender) {
            Common::sendChunkedData(user->clientSocket, 'm', message.toStr(), 100);
        }
    }
    std::lock_guard<std::mutex> lock(consoleMutex);
    std::cout << "Client " << message.sender << ": " << message << std::endl;
}

void Messenger::broadcastMessages(std::mutex& consoleMutex) {
    while (true) {
        std::unique_lock<std::mutex> lock(messageQueueMutex);
        messageAvailableCondition.wait(lock, [&](){ return !messageQueue.empty(); });
        std::cout << "Broadcast Messages: Woke up, processing messages..." << std::endl;

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
    std::cout << "Added message to queue" << std::endl;
}

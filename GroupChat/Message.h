#pragma once
#include <iostream>
#include "MessageType.h"
#include "Room.h"

struct Message {
    std::string sender;
    SOCKET senderSocket;
    std::string message;
    MessageType type;
    std::shared_ptr<Room> room;

    Message(): type(Text) {}

    Message(std::string message, std::string sender, SOCKET senderSocket, std::shared_ptr<Room> room, MessageType type = Text):
        message(std::move(message)), sender(std::move(sender)), senderSocket(senderSocket), room(std::move(room)), type(type) {}

    Message(const Message& other):
        room(other.room), sender(other.sender), senderSocket(other.senderSocket), message(other.message), type(other.type) {}

    Message(Message&& other) noexcept:
        room(std::move(other.room)), sender(std::move(other.sender)), senderSocket(other.senderSocket),
        message(std::move(other.message)), type(other.type) {}

    Message& operator=(const Message& other) {
        if (this != &other) {
            room = other.room;
            sender = other.sender;
            senderSocket = other.senderSocket;
            message = other.message;
            type = other.type;
        }
        return *this;
    }

    Message& operator=(Message&& other) noexcept {
        if (this != &other) {
            room = std::move(other.room);
            sender = std::move(other.sender);
            senderSocket = other.senderSocket;
            message = std::move(other.message);
            type = other.type;
        }
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const Message& ms) {
        os << ms.sender << ": " << ms.message;
        return os;
    }

    std::string toStr() const {
        std::string str;
        switch (type) {
            case Text:
                str = sender + ": " + message;
                break;
            case FileRequest:
                break;
            case File:
                str = sender + ": FILE " + message;
                break;
            case Image:
                break;
            default:
                break;
        }
        return str;
    }

};

#pragma once
#include <iostream>
#include "MessageType.h"

struct Message {
    std::string room;
    std::string sender;
    std::string message;
    MessageType type;

    Message() : type(Text) {}

    Message(std::string message, std::string sender, std::string room, MessageType type = Text) :
        message(std::move(message)), sender(std::move(sender)), room(std::move(room)), type(type) {}

    Message(const Message& other) :
        room(other.room), sender(other.sender), message(other.message), type(other.type) {}

    Message(Message&& other) noexcept :
        room(std::move(other.room)), sender(std::move(other.sender)),
        message(std::move(other.message)), type(other.type) {}

    Message& operator=(const Message& other) {
        if (this != &other) {
            room = other.room;
            sender = other.sender;
            message = other.message;
            type = other.type;
        }
        return *this;
    }

    Message& operator=(Message&& other) noexcept {
        if (this != &other) {
            room = std::move(other.room);
            sender = std::move(other.sender);
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
        std::string str = sender + ": " + message;
        return str;
    }

};

#pragma once
#include <iostream>
#include <WinSock2.h>
#include "UserState.h"

struct User {
    std::string room;
    std::string username;
    SOCKET clientSocket;
    UserState state;

    User(std::string username, SOCKET clientSocket): username(std::move(username)), clientSocket(clientSocket), state(Connected) {
        if (this->username == "") state = Disconnected;
    }

    User(const User& other) : room(other.room), username(other.username), clientSocket(other.clientSocket), state(other.state) {}

    User(User&& other) noexcept
        : room(std::move(other.room)), username(std::move(other.username)), clientSocket(other.clientSocket), state(other.state) {
        other.clientSocket = INVALID_SOCKET;
    }

    ~User() {
        if (clientSocket != INVALID_SOCKET) {
            closesocket(clientSocket);
        }
    }

    User& operator=(const User& other) {
        if (this != &other) {
            room = other.room;
            username = other.username;
            clientSocket = other.clientSocket;
            state = other.state;
        }
        return *this;
    }

    User& operator=(User&& other) noexcept {
        if (this != &other) {
            room = std::move(other.room);
            username = std::move(other.username);
            clientSocket = other.clientSocket;
            state = other.state;
            other.clientSocket = INVALID_SOCKET;
        }
        return *this;
    }
};

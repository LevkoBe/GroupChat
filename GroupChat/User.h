#pragma once
#include <iostream>
#include <WinSock2.h>
#include "UserState.h"

struct User {
	std::string room;
	std::string username;
	SOCKET clientSocket;
	UserState state;

	User(std::string username, SOCKET clientSocket) : username(username), clientSocket(clientSocket), state(Newcomer) {};
};


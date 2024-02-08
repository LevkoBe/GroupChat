#pragma once
#include <iostream>
#include <vector>
#include "User.h"
#include "Message.h"

struct Room {
	std::string password;
	std::string groupName;
	std::vector<User> users;
	std::vector<Message> messageHistory;

	Room(std::string groupName, std::string password="") : groupName(groupName), password(password) {};

	~Room() {
		// delete folder
	}
};


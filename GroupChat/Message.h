#pragma once
#include <iostream>
#include "MessageType.h"

struct Message {
	std::string room;
	std::string sender;
	std::string message;
	MessageType type;

	Message(std::string message, std::string sender, std::string room, MessageType type=Text) :
		message(message), sender(sender), room(room), type(type) {};

};


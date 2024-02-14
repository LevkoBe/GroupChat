#pragma once
#include "Room.h"
#include "Common.h"
#include "Message.h"
#include <queue>
#include <mutex>
#include <thread>
#include <WinSock2.h>

class Messenger {
	std::thread messengerThread;
	std::mutex messageQueueMutex;
	std::condition_variable messageAvailableCondition;
public:
	Messenger(std::mutex& consoleMutex) {
		messengerThread = std::thread(&Messenger::broadcastMessages, this, std::ref(consoleMutex));
		//messengerThread = std::thread(broadcastMessages, consoleMutex);
	}

	void broadcastMessage(const Message& message, std::mutex& consoleMutex);

	void broadcastMessages(std::mutex& consoleMutex);

	void addMessageToQueue(std::shared_ptr<Message> message);

	~Messenger() {
		if (messengerThread.joinable()) {
			messengerThread.join();
		}
	}
};


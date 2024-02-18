#pragma once
#include <mutex>
#include <condition_variable>

extern std::mutex recvLocker;
extern std::condition_variable answerRequired;

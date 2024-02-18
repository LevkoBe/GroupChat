#include "Globals.h"

std::mutex recvLocker;
std::condition_variable answerRequired;
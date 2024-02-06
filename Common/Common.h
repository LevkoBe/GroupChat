#pragma once
#include <WinSock2.h>
#include <iostream>

class Common
{
public:
	static bool errorMessage(const std::string& message);

	static bool sendChunkedData(SOCKET clientSocket, const char operationType, const std::string& messageStr, int chunkSize = 100);

	static std::string receiveChunkedData(SOCKET clientSocket);

	static char receiveOptionType(SOCKET clientSocket);
};


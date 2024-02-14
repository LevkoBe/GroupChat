#pragma once
#include <mutex>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <WinSock2.h>

namespace fs = std::filesystem;

class Common
{
public:
	static bool errorMessage(const std::string& message);

	// basic send-receive
	static bool sendChunkedData(SOCKET clientSocket, const char operationType, const std::string& messageStr, int chunkSize = 100);

	static std::string receiveChunkedData(SOCKET clientSocket);

	static char receiveOptionType(SOCKET clientSocket);

	// file handling
	static bool sendFile(SOCKET clientSocket, const std::string& filepath);

	static bool createFile(const std::string& filepath);

	static bool appendToFile(const std::string& filepath);

	static bool remove(const std::string& filepath);

	// helpers
	static std::vector<std::string> splitStringInTwo(const std::string& str, char delimiter = '\n');

	static std::string fullPath(const std::string& filename, const std::string& username);
};


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
	static bool sendFile(SOCKET clientSocket, const std::string& filename, const std::string& pathAdd="");

	static bool createFile(std::string& content, const std::string& pathAdd="");

	static bool appendToFile(std::string& content, const std::string& pathAdd="");

	static bool removeFolderContents(const std::string& filepath);

	static std::string getFirstFile(const std::string& folderPath);

	// helpers
	static std::vector<std::string> splitStringInTwo(const std::string& str, char delimiter = '\n');
};


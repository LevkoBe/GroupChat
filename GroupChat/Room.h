#pragma once
#include "User.h"
#include "Message.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

struct Room {
    std::string password;
    std::string groupName;
    std::vector<User> users;
    std::vector<Message> messageHistory;

    Room(std::string groupName, std::string password = "") : groupName(groupName), password(password) {
        std::string folderPath = "serverFolder/" + groupName;
        if (fs::exists(folderPath)) {
            std::cout << "Folder '" << folderPath << "' already exists.\n";
        }
        else {
            if (fs::create_directory(folderPath)) {
                std::cout << "Folder '" << folderPath << "' created successfully.\n";
            }
            else {
                std::cout << "Failed to create folder '" << folderPath << "'.\n";
            }
        }
    }

    Room(Room&& room) noexcept
        : password(std::move(room.password)), groupName(std::move(room.groupName)),
        users(std::move(room.users)), messageHistory(std::move(room.messageHistory)) {}

    Room(const Room& other)
        : password(other.password), groupName(other.groupName),
        users(other.users), messageHistory(other.messageHistory) {
    }

    ~Room() {
        std::string folderPath = "serverFolder/" + groupName;
        try {
            if (fs::exists(folderPath)) {
                fs::remove_all(folderPath);
                std::cout << "Folder '" << folderPath << "' deleted successfully.\n";
            }
            else {
                std::cout << "Folder '" << folderPath << "' does not exist.\n";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error deleting folder '" << folderPath << "': " << e.what() << std::endl;
        }
    }

    Room& operator=(const Room& other) {
        if (this != &other) {
            password = other.password;
            groupName = other.groupName;
            users = other.users;
            messageHistory = other.messageHistory;
        }
        return *this;
    }

    Room& operator=(Room&& other) noexcept {
        if (this != &other) {
            password = std::move(other.password);
            groupName = std::move(other.groupName);
            users = std::move(other.users);
            messageHistory = std::move(other.messageHistory);
        }
        return *this;
    }
};


#include "UserService.h"
#include <iostream>
#include <winsock2.h>
#include <map>

std::string generateSessionID() {
    static int counter = 1;
    return "session" + std::to_string(counter++);
}

bool UserService::userExists(const std::string &username, SOCKET client_socket) {
    if (UserDAO::userExists(username)) {
        return true;
    } else {
        const std::string error = "Username does not exist. Do you want to register? (yes/no)\n";
        send(client_socket, error.c_str(), error.length(), 0);
        return false;
    }
}

bool UserService::loginUser(SOCKET client_socket) {
    char buffer[1024] = {0};
    std::string username, password;

    // Ask for username
    send(client_socket, "Enter username: ", 16, 0);
    recv(client_socket, buffer, 1024, 0);
    username = std::string(buffer).substr(0, std::string(buffer).find('\n'));

    if (!userExists(username, client_socket)) {
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, 1024, 0);  // Wait for user input (yes/no)

        if (std::string(buffer).find("yes") != std::string::npos) {
            registerUser(client_socket);
            return false;
        } else {
            std::string retry = "Do you want to try logging in again? (yes/no)\n";
            send(client_socket, retry.c_str(), static_cast<int>(retry.length()), 0);
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, 1024, 0);

            if (std::string(buffer).find("yes") != std::string::npos) {
                return loginUser(client_socket);  // Recursively try login again
            } else {
                std::string disconnect = "Disconnecting...\n";
                send(client_socket, disconnect.c_str(), disconnect.length(), 0);
                return false;
            }
        }
    }

    // Ask for password
    send(client_socket, "Enter password: ", 16, 0);
    memset(buffer, 0, sizeof(buffer));
    recv(client_socket, buffer, 1024, 0);
    password = std::string(buffer).substr(0, std::string(buffer).find('\n'));

    if (UserDAO::isValidUser(username, password)) {
        std::string success = "Login successful!\n";
        send(client_socket, success.c_str(), success.length(), 0);
        return true;
    } else {
        std::string retry = "Invalid password. Do you want to try logging in again? (yes/no)\n";
        send(client_socket, retry.c_str(), retry.length(), 0);
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, 1024, 0);

        if (std::string(buffer).find("yes") != std::string::npos) {
            return loginUser(client_socket);  // Recursively try login again
        } else {
            std::string disconnect = "Disconnecting...\n";
            send(client_socket, disconnect.c_str(), disconnect.length(), 0);
            return false;
        }
    }
}

void UserService::registerUser(SOCKET client_socket) {
    char buffer[1024] = {0};
    std::string username, password;

    // Ask for username
    send(client_socket, "Enter a username to register: ", 30, 0);
    recv(client_socket, buffer, 1024, 0);
    username = std::string(buffer).substr(0, std::string(buffer).find('\n'));

    if (UserDAO::userExists(username)) {
        std::string exists = "Username already exists. Do you want to try logging in? (yes/no)\n";
        send(client_socket, exists.c_str(), exists.length(), 0);
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, 1024, 0);

        if (std::string(buffer).find("yes") != std::string::npos) {
            loginUser(client_socket);
        } else {
            registerUser(client_socket);  // Recursively ask for new username
        }
    } else {
        // Ask for password
        send(client_socket, "Enter a password to register: ", 30, 0);
        memset(buffer, 0, sizeof(buffer));
        recv(client_socket, buffer, 1024, 0);
        password = std::string(buffer).substr(0, std::string(buffer).find('\n'));

        // Create new User and add to database
        std::unique_ptr<User> newUser = std::make_unique<User>(username,password);// Assume password is hashed before setting

        if (UserDAO::addUser(*newUser) >= 0) {
            std::string success = "Registration successful! Do you want to log in? (yes/no) \n";
            send(client_socket, success.c_str(), success.length(), 0);
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, 1024, 0);
            if (std::string(buffer).find("yes") != std::string::npos) {
                loginUser(client_socket);
            } else {
                std::string disconnect = "Disconnecting...\n";
                send(client_socket, disconnect.c_str(), disconnect.length(), 0);
            }
        } else {
            std::string error = "Registration failed. Please try again later. Disconnecting...\n";
            send(client_socket, error.c_str(), error.length(), 0);
        }
    }
}

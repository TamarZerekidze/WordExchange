//
// Created by takusi on 9/2/2024.
//
#include <winsock2.h>
#include "Handler.h"
#include "UserDAO.h"
// Concrete Handlers
class CheckUserExistsHandler : public Handler {
public:
    int handle(SOCKET client_socket, const std::string &username) override {
        if (!UserDAO::userExists(username)) {
            const std::string error = "Username does not exist. Do you want to register? (yes/no)\n";
            send(client_socket, error.c_str(), error.length(), 0);
            char buffer[1024] = {0};
            recv(client_socket, buffer, 1024, 0);
            if (std::string(buffer).find("yes") != std::string::npos) {
                return false;  // Go to registration process
            } else {
                return -1;  // End process
            }
        }
        return Handler::handle(client_socket, username);  // Pass to next handler
    }
};

class ValidatePasswordHandler : public Handler {
public:
    bool handle(SOCKET client_socket, const std::string &username) override {
        send(client_socket, "Enter password: ", 16, 0);
        char buffer[1024] = {0};
        recv(client_socket, buffer, 1024, 0);
        std::string password = std::string(buffer).substr(0, std::string(buffer).find('\n'));

        if (!UserDAO::isValidUser(username, password)) {
            std::string retry = "Invalid password. Do you want to try again? (yes/no)\n";
            send(client_socket, retry.c_str(), retry.length(), 0);
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, 1024, 0);

            if (std::string(buffer).find("yes") != std::string::npos) {
                return false;  // Retry login
            } else {
                return false;  // End process
            }
        }
        std::string success = "Login successful!\n";
        send(client_socket, success.c_str(), success.length(), 0);
        return true;  // Login successful, end chain
    }
};

class CheckUsernameAvailableHandler : public Handler {
public:
    bool handle(SOCKET client_socket, const std::string &username) override {
        if (UserDAO::userExists(username)) {
            const std::string exists = "Username already exists. Do you want to log in? (yes/no)\n";
            send(client_socket, exists.c_str(), exists.length(), 0);
            char buffer[1024] = {0};
            recv(client_socket, buffer, 1024, 0);
            if (std::string(buffer).find("yes") != std::string::npos) {
                return false;  // Redirect to login process
            } else {
                return false;  // End process, or ask for another username
            }
        }
        return Handler::handle(client_socket, username);  // Pass to next handler
    }
};

class CreateUserHandler : public Handler {
public:
    // Overload handle to accept both username and password
    bool handle(SOCKET client_socket, const std::string &username, const std::string &password) {
        std::unique_ptr<User> newUser = std::make_unique<User>(username, password);  // Assume password is hashed before setting

        if (UserDAO::addUser(*newUser) >= 0) {
            const std::string success = "Registration successful! Do you want to log in? (yes/no) \n";
            send(client_socket, success.c_str(), success.length(), 0);
            char buffer[1024] = {0};
            recv(client_socket, buffer, 1024, 0);
            if (std::string(buffer).find("yes") != std::string::npos) {
                return false;  // Redirect to login process
            } else {
                std::string disconnect = "Disconnecting...\n";
                send(client_socket, disconnect.c_str(), disconnect.length(), 0);
            }
        } else {
            const std::string error = "Registration failed. Please try again later. Disconnecting...\n";
            send(client_socket, error.c_str(), error.length(), 0);
        }
        return true;  // End process
    }
};

class GetPasswordForRegistrationHandler : public Handler {
public:
    bool handle(SOCKET client_socket, const std::string &username) override {
        send(client_socket, "Enter a password to register: ", 30, 0);
        char buffer[1024] = {0};
        recv(client_socket, buffer, 1024, 0);
        std::string password = std::string(buffer).substr(0, std::string(buffer).find('\n'));

        // Pass password along with username to next handler
        if (nextHandler) {
            return dynamic_cast<CreateUserHandler*>(nextHandler)->handle(client_socket, username, password);
        }
        return true;
    }
};

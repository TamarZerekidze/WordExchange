
#include <winsock2.h>
#include "UserDAO.h"
#include "UserService.h"
constexpr int BUF_SIZE = 1024;

std::string UserService::prompting(const std::string& from_server, const SOCKET client_socket) {
    send(client_socket, from_server.c_str(), (int)from_server.length(), 0);
    char buffer[BUF_SIZE] = {};
    recv(client_socket, buffer, BUF_SIZE, 0);
    std::string from_client = std::string(buffer).substr(0, std::string(buffer).find('\n'));
    return from_client;
}
std::string generateSessionID() {
    static int counter = 1;
    return "session" + std::to_string(counter++);
}

bool UserService::loginUser(const SOCKET client_socket) {
    //Ask for username
    const std::string username = prompting("Enter username: ", client_socket);
    if (!UserDAO::userExists(username)) {
        const std::string error = "Username does not exist. Do you want to register? (yes/no)\n";
        if (const std::string input = prompting(error, client_socket); input.find("yes") != std::string::npos) {
            registerUser(client_socket);
            return false;
        }
        const std::string retry = "Do you want to try logging in again? (yes/no)\n";
        if (const std::string input = prompting(retry, client_socket); input.find("yes") != std::string::npos) {
            return loginUser(client_socket);  // Recursively try login again
        }
        const std::string disconnect = "Disconnecting...\n";
        send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
        return false;
    }

    // Ask for password
    const std::string password = prompting("Enter password: ", client_socket);
    if (UserDAO::isValidUser(username, password)) {
        const std::string success = "Login successful!\n";
        send(client_socket, success.c_str(), (int)success.length(), 0);
        return true;
    }
    const std::string retry = "Invalid password. Do you want to try logging in again? (yes/no)\n";
    if (const std::string input = prompting(retry, client_socket); input.find("yes") != std::string::npos)
        return loginUser(client_socket);
    const std::string disconnect = "Disconnecting...\n";
    send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
    return false;
}

void UserService::registerUser(const SOCKET client_socket) {
    // Ask for username
const std::string username = prompting("Enter username to register: ", client_socket);
    if (UserDAO::userExists(username)) {
        const std::string exists = "Username already exists. Do you want to try logging in? (yes/no)\n";
        if (const std::string input = prompting(exists, client_socket); input.find("yes") != std::string::npos) {
            loginUser(client_socket);
        } else {
            registerUser(client_socket);  // Recursively ask for new username
        }
    } else {
        // Ask for password
        const std::string password = prompting("Enter a password to register: ", client_socket);
        // Create new User and add to database
        auto newUser = std::make_unique<User>(username, password);
        if (UserDAO::addUser(*newUser) >= 0) {
            const std::string success = "Registration successful! Do you want to log in? (yes/no) \n";
            if (const std::string input = prompting(success, client_socket); input.find("yes") != std::string::npos) {
                loginUser(client_socket);
            } else {
                std::string disconnect = "Disconnecting...\n";
                send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
            }
        } else {
            std::string error = "Registration failed. Please try again later. Disconnecting...\n";
            send(client_socket, error.c_str(), (int)error.length(), 0);
        }
    }
}


#include <winsock2.h>
#include <unordered_set>
#include <thread>
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

std::string UserService::loginUser(const SOCKET client_socket,  std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex) {
    //Ask for username
    std::string username = prompting("Enter username: ", client_socket);
    if (!UserDAO::userExists(username)) {
        const std::string error = "Username does not exist. Do you want to register? (yes/no)\n";
        if (const std::string input = prompting(error, client_socket); input.find("yes") != std::string::npos) {
            registerUser(client_socket, loggedInUsers, loggedInUserMutex);
            return "";
        }
        const std::string retry = "Do you want to try logging in again? (yes/no)\n";
        if (const std::string input = prompting(retry, client_socket); input.find("yes") != std::string::npos) {
            return loginUser(client_socket, loggedInUsers, loggedInUserMutex);  // Recursively try login again
        }
        const std::string disconnect = "Disconnecting...\n";
        send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
        return "";
    }

    // Ask for password
    const std::string password = prompting("Enter password: ", client_socket);
    const std::string user_salt = UserDAO::getSaltByUsername(username);
    std::string hashed_password = PasswordHasher::hashPassword(password, user_salt);
    bool ans;
    {
        std::lock_guard lock(loggedInUserMutex);
        ans = loggedInUsers.contains(username);
    }
        if (ans) {
            const std::string retry = "User is already logged in. Do you want to try logging in with different account again? (yes/no)\n";
            if (const std::string input = prompting(retry, client_socket); input.find("yes") != std::string::npos)
                return loginUser(client_socket, loggedInUsers, loggedInUserMutex);
            const std::string disconnect = "Disconnecting...\n";
            send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
            return "";
        }

    if (UserDAO::isValidUser(username, hashed_password)) {
        const std::string success = "Login successful!\n";
        send(client_socket, success.c_str(), (int)success.length(), 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return username;
    }
    const std::string retry = "Invalid password. Do you want to try logging in again? (yes/no)\n";
    if (const std::string input = prompting(retry, client_socket); input.find("yes") != std::string::npos)
        return loginUser(client_socket, loggedInUsers, loggedInUserMutex);
    const std::string disconnect = "Disconnecting...\n";
    send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
    return "";
}

std::string UserService::registerUser(const SOCKET client_socket,  std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex) {
    // Ask for username
const std::string username = prompting("Enter username to register: ", client_socket);
    if (UserDAO::userExists(username)) {
        const std::string exists = "Username already exists. Do you want to try logging in? (yes/no)\n";
        if (const std::string input = prompting(exists, client_socket); input.find("yes") != std::string::npos) {
            return loginUser(client_socket, loggedInUsers, loggedInUserMutex);
        }
        return registerUser(client_socket, loggedInUsers, loggedInUserMutex);  // Recursively ask for new username
    }
    // Ask for password
    const std::string password = prompting("Enter a password to register: ", client_socket);
    // Create new User and add to database
    const std::string user_salt = this->passwordHash.getStoredSalt();
    std::string hashed_password = this->passwordHash.hashPassword(password);
    auto newUser = std::make_unique<User>(username, hashed_password, user_salt);
    if (UserDAO::addUser(*newUser) >= 0) {
        const std::string success = "Registration successful! Do you want to log in? (yes/no) \n";
        if (const std::string input = prompting(success, client_socket); input.find("yes") != std::string::npos) {
            return loginUser(client_socket, loggedInUsers, loggedInUserMutex);
        }
        const std::string disconnect = "Disconnecting...\n";
        send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
        return "";
    }
    const std::string error = "Registration failed. Please try again later. Disconnecting...\n";
    send(client_socket, error.c_str(), (int)error.length(), 0);
    return "";
}

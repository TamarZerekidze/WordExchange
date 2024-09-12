
#include <winsock2.h>
#include <unordered_set>
#include <thread>
#include "../DAOs/UserDAO.h"
#include "UserService.h"

constexpr int BUF_SIZE = 1024;

// Client input
std::string UserService::prompting(const std::string& from_server, const SOCKET client_socket) {
    send(client_socket, from_server.c_str(), (int)from_server.length(), 0);
    char buffer[BUF_SIZE] = {};
    recv(client_socket, buffer, BUF_SIZE, 0);
    std::string from_client = std::string(buffer).substr(0, std::string(buffer).find('\n'));
    return from_client;
}

std::string UserService::loginUser(const SOCKET client_socket,  std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex, std::mutex& userDaoMutex) {
    //Ask for username
    std::string username = prompting("Enter username: ", client_socket);
    bool notExists;
    {
        std::lock_guard daoLock(userDaoMutex);
        notExists = !UserDAO::userExists(username);
    }
    if (notExists) {
        const std::string error = "Username does not exist. Do you want to register? (yes/no)\n";
        if (const std::string input = prompting(error, client_socket); input.find("yes") != std::string::npos) {
            registerUser(client_socket, loggedInUsers, loggedInUserMutex, userDaoMutex);
            return "";
        }
        const std::string retry = "Do you want to try logging in again? (yes/no)\n";
        if (const std::string input = prompting(retry, client_socket); input.find("yes") != std::string::npos) {
            return loginUser(client_socket, loggedInUsers, loggedInUserMutex, userDaoMutex);  // Recursively try login again
        }
        const std::string disconnect = "Disconnecting...\n";
        send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
        return "";
    }

    // Ask for password
    const std::string password = prompting("Enter password: ", client_socket);
    bool ans;
    {
        std::lock_guard lock(loggedInUserMutex);
        ans = loggedInUsers.contains(username);
    }
        if (ans) {
            const std::string retry = "User is already logged in. Do you want to try logging in with different account again? (yes/no)\n";
            if (const std::string input = prompting(retry, client_socket); input.find("yes") != std::string::npos)
                return loginUser(client_socket, loggedInUsers, loggedInUserMutex, userDaoMutex);
            const std::string disconnect = "Disconnecting...\n";
            send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
            return "";
        }
    bool isValid = false;
    {
        std::lock_guard daoLock(userDaoMutex);
        const std::string user_salt = UserDAO::getSaltByUsername(username);
        std::string hashed_password = PasswordHasher::hashPassword(password, user_salt);
        isValid = UserDAO::isValidUser(username, hashed_password);
    }
    if (isValid) {
        const std::string success = "Login successful!\n";
        send(client_socket, success.c_str(), (int)success.length(), 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return username;
    }
    const std::string retry = "Invalid password. Do you want to try logging in again? (yes/no)\n";
    if (const std::string input = prompting(retry, client_socket); input.find("yes") != std::string::npos)
        return loginUser(client_socket, loggedInUsers, loggedInUserMutex, userDaoMutex);
    const std::string disconnect = "Disconnecting...\n";
    send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
    return "";
}

std::string UserService::registerUser(const SOCKET client_socket,  std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex,  std::mutex& userDaoMutex) {
    // Ask for username
const std::string username = prompting("Enter username to register: ", client_socket);
    bool exists;
    {
    std::lock_guard daoLock(userDaoMutex);
    exists = UserDAO::userExists(username);
    }
    if (exists) {
        const std::string existsMsg = "Username already exists. Do you want to try logging in? (yes/no)\n";
        if (const std::string input = prompting(existsMsg, client_socket); input.find("yes") != std::string::npos) {
            return loginUser(client_socket, loggedInUsers, loggedInUserMutex, userDaoMutex);
        }
        return registerUser(client_socket, loggedInUsers, loggedInUserMutex, userDaoMutex);  // Recursively ask for new username
    }
    // Ask for password
    const std::string password = prompting("Enter a password to register: ", client_socket);
    // Create new User and add to database
    const std::string user_salt = this->passwordHash.getStoredSalt();
    std::string hashed_password = this->passwordHash.hashPassword(password);
    auto newUser = std::make_unique<User>(username, hashed_password, user_salt);
    long long id;
    {
    std::lock_guard daoLock(userDaoMutex);
    id = UserDAO::addUser(*newUser);
    }
    if (id >= 0) {
        const std::string success = "Registration successful! Do you want to log in? (yes/no) \n";
        if (const std::string input = prompting(success, client_socket); input.find("yes") != std::string::npos) {
            return loginUser(client_socket, loggedInUsers, loggedInUserMutex, userDaoMutex);
        }
        const std::string disconnect = "Disconnecting...\n";
        send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
        return "";
    }
    const std::string error = "Registration failed. Please try again later. Disconnecting...\n";
    send(client_socket, error.c_str(), (int)error.length(), 0);
    return "";
}

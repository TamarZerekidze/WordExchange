#pragma once
#include <string>
#include <winsock2.h>
#include "PasswordHasher.h"
#include <memory>

/**
 * @class UserService
 * @brief Manages user login and registration processes using a Chain of Responsibility pattern.
 *
 * The `UserService` class:
 * - Implements `loginUser` and `registerUser` methods to handle user authentication and registration.
 *   - **loginUser**: Validates user credentials by checking the username and password, and then verifies the user’s existence.
 *   - **registerUser**: Manages user registration by ensuring a unique username, creating a new `User` object, and setting the password.
 *
 * It utilizes:
 * - `PasswordHasher` for hashing and verifying passwords.
 * - Helper methods for prompting and receiving input from the server.
 *
 * The class interacts with shared resources such as logged-in users and database access through mutexes to ensure thread safety.
 */

class UserService {
private:
    PasswordHasher passwordHash;
public:
    UserService() = default;
    ~UserService() = default;
    static std::string prompting(const std::string& from_server, SOCKET client_socket);
    std::string loginUser(SOCKET client_socket,  std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex,  std::mutex& userDaoMutex);
    std::string registerUser(SOCKET client_socket,  std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex,  std::mutex& userDaoMutex);
};
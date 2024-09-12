#pragma once
#include "../Objects/User.h"

/**
 * @class UserDAO
 * @brief Manages database operations for user-related data using SQLite commands.
 *
 * The `UserDAO` class:
 * - Adds new users to the database.
 * - Retrieves a `User` object based on the username if it exists.
 * - Checks for the existence of a user by username.
 * - Validates user credentials during login.
 */

class UserDAO{
public:
    UserDAO();
    static bool userExists(const std::string &username);
    static long long addUser(User& user);
    static bool removeUser(long long uid);
    [[nodiscard]] static std::unique_ptr<User> getUserByUsername(const std::string &username);
    [[nodiscard]] static std::string getSaltByUsername(const std::string &username);
    static bool isValidUser(const std::string &username, const std::string &pass);
    /* std::vector<User> getAllUsers() override; */
};


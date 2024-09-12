#pragma once
#include "User.h"

/* UserDAO class is Database Access Object that uses sqlite commands to retrieve or add information about users in DB.
 * it adds new user, returns User object with username if it exists, checks if user exists via username and validates user
 * during login */

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


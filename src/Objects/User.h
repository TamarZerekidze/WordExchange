#pragma once

#include <string>
#include <chrono>
#include <ctime>

/**
 * @class User
 * @brief Represents a user with attributes such as username, hashed password, registration date, and user ID.
 *
 * The `User` class manages user data including:
 * - `username`: The username of the user.
 * - `hashed_password`: The hashed password for security.
 * - `registration_date`: When the user was created.
 * - `user_id`: Unique identifier from the database.
 *
 * Provides getter and setter methods for these attributes and an equality operator to compare `User` objects.
 */

class User{
private:
    long long id = -1;
    std::string username;
    std::string password;
    std::string salt;
    std::time_t dateAdded = 0;

public:
    User();
    User(std::string name, std::string password, std::string salt);
    User(std::string name, std::string password, std::string salt, std::time_t dateAdded);

    [[nodiscard]] long long getId() const ;
    [[nodiscard]] std::string getUsername() const ;
    [[nodiscard]] std::string getPassword() const;
    [[nodiscard]] std::string getSalt() const;
    [[nodiscard]] time_t getTimeAdded() const;
/*TODO: is it not a problem that username and password is passed in reference, bcause what if they are deleted in main? */
    void setUserId(long long id);
    void setUsername(const std::string &name);
    void setPassword(const std::string &pass);
    void setSalt(const std::string &salt);
    void setDateAdded(std::time_t dateAdded);

    bool operator==(const User &o) const;
};



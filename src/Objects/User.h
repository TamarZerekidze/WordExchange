#pragma once

#include <string>
#include <chrono>
#include <ctime>

/* User class represent user object,it has attributes as fields like username, hashed password, when user was created,
 * aka registration date and user_id(database attribute). it has getter/setter functions to retrieve each field or
 * change,reset it. and equality operator that checks if two User objects are the same.*/

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



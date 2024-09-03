#pragma once

#include <string>
#include <chrono>
#include <ctime>

/*
class IUser {
public:
    virtual ~IUser() = default;
    [[nodiscard]] virtual long long getId() const = 0;
    [[nodiscard]] virtual std::string getUsername() const = 0;
    [[nodiscard]] virtual std::string getPassword() const = 0;
    [[nodiscard]] virtual time_t getTimeAdded() const = 0;

    virtual void setUserId(long long id) = 0;
    virtual void setUsername(const std::string &name) = 0;
    virtual void setPassword(const std::string &pass) = 0;
    virtual void setDateAdded(time_t dateAdded) = 0;
}; */

class User{
private:
    long long id = -1;
    std::string username;
    std::string password ;
    std::time_t dateAdded = 0;

public:
    User();
    User(std::string name, std::string password);
    User(std::string name, std::string password, std::time_t dateAdded);

    [[nodiscard]] long long getId() const ;
    [[nodiscard]] std::string getUsername() const ;
    [[nodiscard]] std::string getPassword() const;
    [[nodiscard]] time_t getTimeAdded() const;
/*TODO: is it not a problem that username and password is passed in reference, bcause what if they are deleted in main? */
    void setUserId(long long id);
    void setUsername(const std::string &name);
    void setPassword(const std::string &pass);
    void setDateAdded(std::time_t dateAdded);

    bool operator==(const User &o) const;
};



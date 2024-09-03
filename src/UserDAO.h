#pragma once
#include "User.h"
/*
class IUserDAO {
public:
    virtual ~IUserDAO() = default;
    virtual bool userExists(const std::string &username) = 0;
    virtual long long addUser(User& user) = 0;
    virtual bool removeUser(long long uid) = 0;
    [[nodiscard]] virtual User getUserByUsername(const std::string &username) = 0;
    virtual bool isValidUser(const std::string &username, const std::string &pass) = 0;
    /* virtual std::vector<User> getAllUsers() = 0; */
 /*}; */
class UserDAO{
public:
    UserDAO();
    static bool userExists(const std::string &username);

    static long long addUser(User& user);
    static bool removeUser(long long uid);
    [[nodiscard]] static std::unique_ptr<User> getUserByUsername(const std::string &username);

    static bool isValidUser(const std::string &username, const std::string &pass);
    /* std::vector<User> getAllUsers() override; */
};


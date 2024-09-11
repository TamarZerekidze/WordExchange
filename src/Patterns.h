
#pragma once

#include <memory>
#include "UserService.h"

/* In this file there are several classes which use design patters principle and userService class uses them.
 * first is Strategy pattern, which has base class - abstract class(interface) which has pure virtual function "execute"
 * as I have different operations (login and register) each of them implement execute function their way.
 * however, both these objects are IUserOperation instances during runtime compilation.
 * I also use Factory pattern, that creates one of these objects based on input.
 */

class IUserOperation {
public:
    virtual std::string execute(SOCKET client_socket, std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex,  std::mutex& userDaoMutex) = 0;
    virtual ~IUserOperation() = default;
};

class LoginOperation final : public IUserOperation {
private:
    std::shared_ptr<UserService> userService;

public:
    explicit LoginOperation(std::shared_ptr<UserService> service) : userService(std::move(service)) {}

    std::string execute(const SOCKET client_socket,  std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex,  std::mutex& userDaoMutex) override {
        return userService->loginUser(client_socket, loggedInUsers, loggedInUserMutex,  userDaoMutex);
    }
};

class RegisterOperation final : public IUserOperation {
private:
    std::shared_ptr<UserService> userService;

public:
    explicit RegisterOperation(std::shared_ptr<UserService> service) : userService(std::move(service)) {}

    std::string execute(const SOCKET client_socket,  std::unordered_set<std::string>& loggedInUsers, std::mutex& loggedInUserMutex,  std::mutex& userDaoMutex) override {
        return userService->registerUser(client_socket, loggedInUsers, loggedInUserMutex,  userDaoMutex);
    }
};

class UserOperationFactory {
public:
    static std::unique_ptr<IUserOperation> createOperation(const std::string &input, const std::shared_ptr<UserService>& userService) {
        if (input.find("login") != std::string::npos) {
            return std::make_unique<LoginOperation>(userService);
        }
        if (input.find("register") != std::string::npos) {
            return std::make_unique<RegisterOperation>(userService);
        }
        return nullptr;
    }
};
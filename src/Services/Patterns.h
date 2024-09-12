#pragma once
#include <memory>
#include "UserService.h"

/**
 * @file
 * @brief Contains classes that implement design patterns, utilized by the `UserService` class.
 *
 * This file demonstrates:
 * - **Strategy Pattern**: An abstract base class (interface) defines a pure virtual `execute` function.
 *   Concrete implementations provide specific behaviors for operations like login and registration.
 *   Both implementations are treated as `IUserOperation` instances at runtime.
 * - **Factory Pattern**: A factory creates instances of `IUserOperation` based on input, selecting the appropriate strategy.
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
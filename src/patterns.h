
#ifndef PATTERNS_H
#define PATTERNS_H

#include <memory>
#include "UserService.h"
class IUserOperation {
public:
    virtual void execute(SOCKET client_socket) = 0;
    virtual ~IUserOperation() = default;
};

class LoginOperation final : public IUserOperation {
private:
    std::shared_ptr<UserService> userService;

public:
    explicit LoginOperation(std::shared_ptr<UserService> service) : userService(std::move(service)) {}

    void execute(const SOCKET client_socket) override {
        userService->loginUser(client_socket);
    }
};

class RegisterOperation final : public IUserOperation {
private:
    std::shared_ptr<UserService> userService;

public:
    explicit RegisterOperation(std::shared_ptr<UserService> service) : userService(std::move(service)) {}

    void execute(const SOCKET client_socket) override {
        userService->registerUser(client_socket);
    }
};

class UserOperationFactory {
public:
    static std::unique_ptr<IUserOperation> createOperation(const std::string &input, const std::shared_ptr<UserService>& userService) {
        if (input.find("login") != std::string::npos) {
            return std::make_unique<LoginOperation>(userService);
        } else if (input.find("register") != std::string::npos) {
            return std::make_unique<RegisterOperation>(userService);
        }
        return nullptr;
    }
};

#endif //PATTERNS_H

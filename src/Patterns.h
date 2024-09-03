
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
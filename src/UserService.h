#pragma once
#include <string>
#include <winsock2.h>

/* UserService is a helper class that has two functions, loginUser and registerUser. these two functions
 * contain prompts of Server and have Chain of Responsibility pattern, as validation for login is two steps,
 * first username checking and then password checking. similarly registerUser has 3 steps, new unique username checking,
 * creating User and then password. helper methods for prompt sending/receiving is also used.*/

class UserService {
public:
    UserService() = default;

    static std::string prompting(const std::string& from_server, SOCKET client_socket);
    bool loginUser(SOCKET client_socket);
    void registerUser(SOCKET client_socket);
};



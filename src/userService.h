#ifndef USERSERVICE_H
#define USERSERVICE_H

#include "UserDAO.h"
#include "User.h"
#include <winsock2.h>
#include <map>

class UserService {
public:
    UserService() = default;

    bool loginUser(SOCKET client_socket);
    void registerUser(SOCKET client_socket);
    bool userExists(const std::string& username, SOCKET client_socket);
};

#endif // USERSERVICE_H


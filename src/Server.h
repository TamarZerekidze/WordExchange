#pragma once

#include <winsock2.h>
#include <unordered_set>
#include "UserService.h"
#include "Patterns.h"

/*Server class creates server socket binds it, and makes it listen to a port for incoming connections from client.
 *this class administers every interaction with database, upon the requests of Client. it uses UserService class
 * for business logic. has a start() function and handleClient() which is run in different threads.*/

class Server {
private:
    SOCKET server_socket;
    std::shared_ptr<UserService> userService;
    UserOperationFactory operationFactory{};
    struct sockaddr_in server_addr{};
    std::mutex loggedInUserMutex;

public:
    std::unordered_set<std::string> loggedInUsers;
    explicit Server(std::shared_ptr<UserService> service);
    ~Server();

    void start();
    void handleClient(SOCKET client_socket);
};


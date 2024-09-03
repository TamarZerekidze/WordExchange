#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "UserService.h"
#include <map>
#include "patterns.h"
class Server {
private:
    SOCKET server_socket;
    std::shared_ptr<UserService> userService;
    UserOperationFactory operationFactory{};
    struct sockaddr_in server_addr{};

public:
    explicit Server(std::shared_ptr<UserService> service);
    ~Server();

    void start();
    void handleClient(SOCKET client_socket) const;
};

#endif // SERVER_H


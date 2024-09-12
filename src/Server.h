#pragma once

#include <winsock2.h>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <condition_variable>
#include "UserService.h"
#include "Patterns.h"

/*Server class creates server socket binds it, and makes it listen to a port for incoming connections from client.
 *this class administers every interaction with database, upon the requests of Client. it uses UserService class
 * for business logic. has a start() function and handleClient() which is run in different threads.*/

class Server {
private:
    SOCKET server_socket;
    std::shared_ptr<UserService> userService;
    std::mutex userDaoMutex;
    UserOperationFactory operationFactory{};
    struct sockaddr_in server_addr{};
    std::mutex loggedInUserMutex;
    std::unordered_set<std::string> loggedInUsers;
    std::queue<std::pair<std::string, SOCKET> > matchmakingQueue;
    std::mutex matchmakingMutex;
    std::condition_variable matchmakingCV;
    std::set< std::pair<std::string, SOCKET> > playingSet;
    std::mutex playingMutex;
    std::condition_variable playingCV;
    std::mutex gameDaoMutex;

public:
    explicit Server(std::shared_ptr<UserService> service);
    ~Server();

    void start();

    void startMatchmaking();

    void playLoop(const std::string& uname, SOCKET client_socket);

    void handleClient(SOCKET client_socket);

    void startGameSession(std::pair<std::string, SOCKET> player1, std::pair<std::string, SOCKET> player2);
};


#pragma once

#include <winsock2.h>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <condition_variable>
#include "Services/UserService.h"
#include "Services/Patterns.h"

/**
 * @class Server
 * @brief Manages server operations including client connections, matchmaking, and game sessions.
 *
 * The `Server` class:
 * - Creates and listens on a server socket for client connections.
 * - Handles client requests and interacts with the database via `UserService`.
 * - Provides functionalities for matchmaking, game management, and viewing leaderboards (global and personal).
 * - Runs `start()` to initialize the server and `handleClient(SOCKET client_socket)` for client communication in separate threads.
 */

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


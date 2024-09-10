#include <iostream>
#include <string>
#include <set>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "Server.h"
#include "Patterns.h"
// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
constexpr int BUF_SIZE = 1024;

std::queue<std::pair<std::string, SOCKET> > matchmakingQueue;
std::mutex matchmakingMutex;
std::condition_variable matchmakingCV;
std::set< std::pair<std::string, SOCKET> > playingSet;
std::mutex playingMutex;
std::condition_variable playingCV;



Server::Server(std::shared_ptr<UserService> service) : userService(std::move(service)) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        exit(EXIT_FAILURE);
    }

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Define server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(55000);

    // Bind the socket to the port
    if (bind(server_socket, (struct sockaddr *)(&server_addr), sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    // Start listening for client connections
    if (listen(server_socket, SOMAXCONN ) == SOCKET_ERROR) {
        std::cerr << "Listen failed\n";
        closesocket(server_socket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port 55000\n";
}

Server::~Server() {
    closesocket(server_socket);
    WSACleanup();
}

void Server::start() {
    SOCKET client_socket;
    while ((client_socket = accept(server_socket, nullptr, nullptr)) != INVALID_SOCKET) {
        std::thread clientThread(&Server::handleClient, this, client_socket);
        clientThread.detach();
    }
}

void Server::startMatchmaking() {
    while (true) {
        std::chrono::seconds timeout_duration(30);
        std::unique_lock lock(matchmakingMutex);

        // Wait for at least one player in the queue
        matchmakingCV.wait(lock, [] { return !matchmakingQueue.empty(); });        // Match two players

        if (matchmakingQueue.size() == 1) {
            auto waitingPlayer = matchmakingQueue.front(); // Get the player (username and socket)
            const std::string waitingMsg = "No partner available. Waiting for another player to join...\n";
            send(waitingPlayer.second, waitingMsg.c_str(), (int)waitingMsg.length(), 0);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            // Wait until another player joins the queue
            if (!matchmakingCV.wait_for(lock, timeout_duration, [] { return matchmakingQueue.size() >= 2; })) {
                // Timeout occurred, remove the player from the queue
                matchmakingQueue.pop();
                const std::string timeoutMsg = "No partner found within the timeout. Returning to the main menu.\n";
                send(waitingPlayer.second, timeoutMsg.c_str(), (int)timeoutMsg.length(), 0);
                continue; // Go back to waiting for players
            }
        }


        auto player1 = matchmakingQueue.front();
        matchmakingQueue.pop();
        auto player2 = matchmakingQueue.front();
        matchmakingQueue.pop();

        lock.unlock();

        {
            std::lock_guard gameLock(playingMutex);
            playingSet.insert(player1); // Add player1 to the set
            playingSet.insert(player2); // Add player2 to the set
        }
        playingCV.notify_all();

        std::thread gameThread(&Server::startGameSession, this, player1, player2);
        gameThread.detach();
    }
}

void Server::handleClient(const SOCKET client_socket) {
    const std::string str = "Do you want to login or register? (login/register)\n";
    const std::string input = UserService::prompting(str, client_socket);
    if (std::unique_ptr<IUserOperation> operation = UserOperationFactory::createOperation(input, userService)) {
        const std::string& uname = operation->execute(client_socket, loggedInUsers, loggedInUserMutex);
        if (uname.empty()) {
            closesocket(client_socket);
            return;
        }
        if (!uname.empty()) {
            {
                std::lock_guard lock(loggedInUserMutex);
                this->loggedInUsers.insert(uname);
                std::cout << loggedInUsers.size() << std::endl;
            }
        }
        while (true) {
            const std::string success = "Would you like to find partner?!\n";
            send(client_socket, success.c_str(), (int)success.length(), 0);
            char buffer[BUF_SIZE] = {};
            auto bytes_rec = recv(client_socket, buffer, BUF_SIZE, 0);
            if (bytes_rec <= 0) {
                {
                    std::lock_guard lock(loggedInUserMutex);
                    this->loggedInUsers.erase(uname);
                    std::cout << loggedInUsers.size() << std::endl;
                }
                break;
            }
            if (const std::string from_client = std::string(buffer).substr(0, std::string(buffer).find('\n')); from_client != "yes") {
                {
                    std::lock_guard lock(loggedInUserMutex);
                    this->loggedInUsers.erase(uname);
                    std::cout << loggedInUsers.size() << std::endl;
                }
                const std::string disconnect = "Disconnecting...\n";
                send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
                break;
            }
            {
                std::lock_guard queueLock(matchmakingMutex);
                matchmakingQueue.emplace(uname, client_socket);
                matchmakingCV.notify_one();

            }


            // wait till game session finishes or something happens to opponent
            {
                std::chrono::seconds timeout_duration(40);
                std::unique_lock gameLock(playingMutex);
                auto gameStarting = playingCV.wait_for(gameLock, timeout_duration, [this, uname, client_socket] {
                    return playingSet.contains({uname, client_socket});
                });
                if (!gameStarting) {
                    continue;
                }

                 playingCV.wait(gameLock, [this, uname, client_socket] {
                    return !playingSet.contains({uname, client_socket});
                });
            }
        }
    } else {
        send(client_socket, "Disconnecting...\n", 30, 0);
    }
    closesocket(client_socket);  // Close the client socket after disconnect
}

void Server::startGameSession(std::pair<std::string, SOCKET> player1, std::pair<std::string, SOCKET> player2) {
    SOCKET clientSocket1 = player1.second;
    SOCKET clientSocket2 = player2.second;
    const std::string foundOpponent1 = "Game is starting...your partner is " + player2.first + "\n";
    const std::string foundOpponent2 = "Game is starting...your partner is " + player1.first + "\n";
    send(clientSocket1, foundOpponent1.c_str(), (int)foundOpponent1.length(), 0);
    send(clientSocket2, foundOpponent2.c_str(), (int)foundOpponent2.length(), 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    while(true) {
        fd_set readfds;
        struct timeval timeout{};
        timeout.tv_sec = 30; // Set timeout to 30 seconds
        timeout.tv_usec = 0;

        // Notify both players
        const std::string matchFoundMsg = "Please type the word: \n";
        send(clientSocket1, matchFoundMsg.c_str(), (int)matchFoundMsg.length(), 0);
        send(clientSocket2, matchFoundMsg.c_str(), (int)matchFoundMsg.length(), 0);


        char buffer[BUF_SIZE] = {};
        char buffer2[BUF_SIZE] = {};
        std::string input1;
        std::string input2;

        memset(buffer, 0, BUF_SIZE);
        memset(buffer2, 0, BUF_SIZE);
        int bytesReceived = recv(clientSocket1, buffer, BUF_SIZE, 0);
        int bytesReceived2 = recv(clientSocket2, buffer2, BUF_SIZE, 0);
        if (bytesReceived <= 0) {
            const std::string discMsg = "It seems that " + player1.first +" was disconnected. Game can not proceed.";
            send(clientSocket2, discMsg.c_str(), (int)discMsg.length(), 0);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                std::unique_lock gameLock(playingMutex);
                playingSet.erase(player1);
                playingSet.erase(player2);
            }
            playingCV.notify_all();
            return;
        }
        input1 = std::string(buffer).substr(0, std::string(buffer).find('\n'));
        std::string waitForOtherMsg = "Your word recorded. Waiting for the other player... \n";
        send(clientSocket1, waitForOtherMsg.c_str(), (int)waitForOtherMsg.length(), 0);

        if (bytesReceived2 <= 0) {
            const std::string discMsg = "It seems that " + player2.first +" was disconnected. Game can not proceed.\n";
            send(clientSocket1, discMsg.c_str(), (int)discMsg.length(), 0);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                std::unique_lock gameLock(playingMutex);
                playingSet.erase(player2);
                playingSet.erase(player1);
            }
            playingCV.notify_all();
            return;
        }
        input2 = std::string(buffer2).substr(0, std::string(buffer2).find('\n'));
        std::string waitForOtherMsg2 = "Your word recorded.\n";
        send(clientSocket2, waitForOtherMsg2.c_str(), (int)waitForOtherMsg2.length(), 0);


        // Both players are ready, start the game
        std::this_thread::sleep_for(std::chrono::seconds(1));
        const std::string gameStartMsg = "Both words taken in!\n";
        send(clientSocket1, gameStartMsg.c_str(), (int)gameStartMsg.length(), 0);
        send(clientSocket2, gameStartMsg.c_str(), (int)gameStartMsg.length(), 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));


        if(input1 == input2) {
            const std::string congrats = "Congratulations, both of you choose: '" + input1 + "' you have matched!\n";
            send(clientSocket1, congrats.c_str(), (int)congrats.length(), 0);
            send(clientSocket2, congrats.c_str(), (int)congrats.length(), 0);
            {
                std::unique_lock gameLock(playingMutex);
                playingSet.erase(player2);
                playingSet.erase(player1);
            }
            playingCV.notify_all();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return;
        }
        std::string playerInputs = player1.first;
        playerInputs +=  " : ";
        playerInputs += input1;
        playerInputs +=  ", ";
        playerInputs += player2.first;
        playerInputs +=  " : ";
        playerInputs += input2;
        std::string msg = "Words provided: ";
        msg += playerInputs;
        msg += " do not match! Game continues. \n";
        send(clientSocket1, msg.c_str(), (int)msg.length(), 0);
        send(clientSocket2, msg.c_str(), (int)msg.length(), 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    auto userService = std::make_unique<UserService>();
    Server server(std::move(userService));
    std::thread matchmakingThread(&Server::startMatchmaking, &server);
    matchmakingThread.detach();
    server.start();
    return 0;
}
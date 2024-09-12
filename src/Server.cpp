#include <iostream>
#include <string>
#include <set>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "Server.h"

#include "DAOs/GameDAO.h"
#include "Services/MenuService.h"
#include "Services/Patterns.h"
#include "Objects/Session.h"
// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
constexpr int BUF_SIZE = 2048;

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
        matchmakingCV.wait(lock, [this] { return !matchmakingQueue.empty(); });        // Match two players

        if (matchmakingQueue.size() == 1) {
            auto waitingPlayer = matchmakingQueue.front(); // Get the player (username and socket)
            const std::string waitingMsg = "No partner available. Waiting for another player to join...\n";
            send(waitingPlayer.second, waitingMsg.c_str(), (int)waitingMsg.length(), 0);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            // Wait until another player joins the queue
            if (!matchmakingCV.wait_for(lock, timeout_duration, [this] { return matchmakingQueue.size() >= 2; })) {
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

void Server::playLoop (const std::string& uname, SOCKET client_socket) {
    {
        std::lock_guard queueLock(matchmakingMutex);
        matchmakingQueue.emplace(uname, client_socket);
        matchmakingCV.notify_one();
    }
    // wait till game session finishes or something happens to opponent
    {
        std::chrono::seconds timeout_duration(35);
        std::unique_lock gameLock(playingMutex);
        auto gameStarting = playingCV.wait_for(gameLock, timeout_duration, [this, uname, client_socket] {
            return playingSet.contains({uname, client_socket});
        });
        if (!gameStarting) return;

        playingCV.wait(gameLock, [this, uname, client_socket] {
           return !playingSet.contains({uname, client_socket});
       });
    }
}


void Server::handleClient(const SOCKET client_socket) {
    const std::string str = "Do you want to login or register? (login/register/exit)\n";
    const std::string input = UserService::prompting(str, client_socket);
    if (std::unique_ptr<IUserOperation> operation = UserOperationFactory::createOperation(input, userService)) {
        const std::string& uname = operation->execute(client_socket, loggedInUsers, loggedInUserMutex, userDaoMutex);
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
            const std::string success = "Hello to main menu, what would you like to do? (play/leaderboard/exit)\n";
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
            const std::string from_client = std::string(buffer).substr(0, std::string(buffer).find('\n'));
            if (from_client != "play" && from_client != "leaderboard") {
                {
                    std::lock_guard lock(loggedInUserMutex);
                    this->loggedInUsers.erase(uname);
                    std::cout << loggedInUsers.size() << std::endl;
                }
                const std::string disconnect = "Disconnecting...\n";
                send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
                break;
            }

            if (from_client == "play") {
                playLoop(uname, client_socket);
            }
            if (from_client == "leaderboard") {
                const std::string leaderboard = MenuService::fetchLeaderboard(uname, gameDaoMutex);
                send(client_socket, leaderboard.c_str(), (int) leaderboard.length() , 0);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                const std::string roundMsg = "Would you like to see specific game flow? (yes/no)\n";
                send(client_socket, roundMsg.c_str(), (int)roundMsg.length(), 0);
                char buf[BUF_SIZE] = {};
                recv(client_socket, buf, BUF_SIZE, 0);
                std::string client_response = std::string(buf).substr(0, std::string(buf).find('\n'));
                if (client_response == "yes") {
                    const std::string idMsg = "Please provide game id: \n";
                    send(client_socket, idMsg.c_str(), (int)idMsg.length(), 0);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    memset(buf, 0, BUF_SIZE);
                    recv(client_socket, buf, BUF_SIZE, 0);
                    client_response = std::string(buf).substr(0, std::string(buf).find('\n'));
                    const std::string roundTable = MenuService::fetchGameSession(std::stoi(client_response), gameDaoMutex);
                    send(client_socket, roundTable.c_str(), (int)roundTable.length(), 0);
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }
    } else {
        const std::string disc = "Disconnecting...\n";
        send(client_socket, disc.c_str(), (int) disc.length() , 0);
    }
    closesocket(client_socket);  // Close the client socket after disconnect
}

void sendMessages(const SOCKET client1, const SOCKET client2, const std::string& message) {
    send(client1, message.c_str(), (int)message.length(), 0);
    send(client2, message.c_str(), (int)message.length(), 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void sendMessages(const SOCKET client1, const SOCKET client2, const std::string& message1, const std::string& message2) {
    send(client1, message1.c_str(), (int)message1.length(), 0);
    send(client2, message2.c_str(), (int)message2.length(), 0);
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

void disconnectMessage(const SOCKET client, const std::string& who) {
    const std::string discMsg = "It seems that " + who +" was disconnected. Game can not proceed.";
    send(client, discMsg.c_str(), (int)discMsg.length(), 0);
}

std::string constructMessage(const std::string& pl1, const std::string& input1, const std::string& pl2, const std::string& input2) {
    std::string playerInputs = pl1;
    playerInputs +=  " : ";
    playerInputs += input1;
    playerInputs +=  ", ";
    playerInputs += pl2;
    playerInputs +=  " : ";
    playerInputs += input2;
    std::string msg = "Words provided: ";
    msg += playerInputs;
    msg += " do not match! Game continues. \n";
    return msg;
}


void Server::startGameSession(std::pair<std::string, SOCKET> player1, std::pair<std::string, SOCKET> player2) {
    std::vector<std::pair<std::string, std::string> > rounds;
    const SOCKET clientSocket1 = player1.second;
    const SOCKET clientSocket2 = player2.second;
    const std::string foundOpponent1 = "Game is starting...your partner is " + player2.first + "\n";
    const std::string foundOpponent2 = "Game is starting...your partner is " + player1.first + "\n";
    sendMessages(clientSocket1, clientSocket2, foundOpponent1, foundOpponent2);
    while(true) {
        // Notify both players
        sendMessages(clientSocket1, clientSocket2, "Please type the word: \n");
        char buffer[BUF_SIZE] = {};
        char buffer2[BUF_SIZE] = {};
        std::string input1, input2;

        memset(buffer, 0, BUF_SIZE);
        memset(buffer2, 0, BUF_SIZE);
        int bytesReceived = recv(clientSocket1, buffer, BUF_SIZE, 0);
        int bytesReceived2 = recv(clientSocket2, buffer2, BUF_SIZE, 0);
        auto shouldDisc = bytesReceived <= 0 || bytesReceived2 <= 0;
        if (bytesReceived <= 0) {
            disconnectMessage(clientSocket2, player1.first);
        }
        if (bytesReceived2 <= 0) {
            disconnectMessage(clientSocket1, player2.first);
        }
        if (shouldDisc) {
            {
                std::unique_lock gameLock(playingMutex);
                playingSet.erase(player1);
                playingSet.erase(player2);
            }
            playingCV.notify_all();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return;
        }
        input1 = std::string(buffer).substr(0, std::string(buffer).find('\n'));
        input2 = std::string(buffer2).substr(0, std::string(buffer2).find('\n'));
        sendMessages(clientSocket1, clientSocket2, "Your word recorded.\n" );
        // Both players are ready, start the game
        sendMessages(clientSocket1, clientSocket2, "Both words taken in!\n");
        rounds.emplace_back(input1, input2);
        if(input1 == input2) {
            const std::string congrats = "Congratulations, both of you choose: '" + input1 + "' you have matched!\n";
            sendMessages(clientSocket1, clientSocket2, congrats);
            {
                std::lock_guard sessionLock(gameDaoMutex);
                auto gameSession = Session(player1.first, player2.first, (int)rounds.size());
                GameDAO::addSession(rounds,gameSession);
            }
            {
                std::unique_lock gameLock(playingMutex);
                playingSet.erase(player2);
                playingSet.erase(player1);
            }
            playingCV.notify_all();
            return;
        }
        sendMessages(clientSocket1, clientSocket2, constructMessage(player1.first, input1, player2.first, input2));
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
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include "Server.h"
#include "Patterns.h"
// Link with ws2_32.lib
#pragma comment(lib, "ws2_32.lib")
constexpr int BUF_SIZE = 1024;
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

void Server::handleClient(const SOCKET client_socket) {
    const std::string str = "Do you want to login or register? (login/register)\n";
    const std::string input = UserService::prompting(str, client_socket);
    if (std::unique_ptr<IUserOperation> operation = UserOperationFactory::createOperation(input, userService)) {
        const std::string& uname = operation->execute(client_socket, loggedInUsers, loggedInUserMutex);
        if (!uname.empty()) {
            {
                std::lock_guard lock(loggedInUserMutex);
                this->loggedInUsers.insert(uname);
                std::cout << loggedInUsers.size() << std::endl;
            }
        }
        char buffer[BUF_SIZE] = {};
        auto bytes_rec = recv(client_socket, buffer, BUF_SIZE, 0);
        if (bytes_rec <= 0) {
            {
                std::lock_guard lock(loggedInUserMutex);
                this->loggedInUsers.erase(uname);
                std::cout << loggedInUsers.size() << std::endl;
            }
        } else if (const std::string from_client = std::string(buffer).substr(0, std::string(buffer).find('\n')); from_client != "yes") {
            {
                std::lock_guard lock(loggedInUserMutex);
                this->loggedInUsers.erase(uname);
                std::cout << loggedInUsers.size() << std::endl;
            }
                const std::string disconnect = "Disconnecting...\n";
                send(client_socket, disconnect.c_str(), (int)disconnect.length(), 0);
        }

    } else {
        send(client_socket, "Disconnecting...\n", 30, 0);
    }
    closesocket(client_socket);  // Close the client socket after disconnect
}


int main() {
    auto userService = std::make_unique<UserService>();
    Server server(std::move(userService));
    server.start();
    return 0;
}
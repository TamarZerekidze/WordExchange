#include "sqlite/sqlite3.h"
#include <iostream>
#include "src/DatabaseConnection.h"
#include <ctime>
#include "src/User.h"
#include "src/UserDAO.h"
#include "hash256/sha256.h"
#include <sstream>
#include <random>

void testUserDAO() {

    const std::string username = "tako";
    const std::string password = "dushki";
    const std::time_t dateAdded = std::time(nullptr);
    User newUser(username, password, dateAdded);

    if (UserDAO::userExists(username)) {
        std::cout << "User already exists before adding. Test failed." << std::endl;
    } else {
        std::cout << "User does not exist. Proceeding to add user." << std::endl;
    }

    if (const long long userId = UserDAO::addUser(newUser); userId >= 0) {
        std::cout << "User added successfully with ID: " << userId << std::endl;
    } else {
        std::cout << "Failed to add user. Test failed." << std::endl;
    }

    if (UserDAO::isValidUser(username, password)) {
        std::cout << "User validation successful. Test passed." << std::endl;
    } else {
        std::cout << "User validation failed. Test failed." << std::endl;
    }

    if (!UserDAO::isValidUser(username, "wrong")) {
        std::cout << "Invalid password correctly rejected. Test passed." << std::endl;
    } else {
        std::cout << "User validation with wrong password succeeded. Test failed." << std::endl;
    }

    if (const std::unique_ptr<User> fetchedUser = UserDAO::getUserByUsername(username); fetchedUser->getUsername() == username) {
        std::cout << fetchedUser->getId() << std::endl;
        std::cout << "User fetched successfully by username. Test passed." << std::endl;
    } else {
        std::cout << "Failed to fetch user by username. Test failed." << std::endl;
    }

}

std::string generate_salt(size_t length = 16) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    return ss.str();
}

std::string hash_password(const std::string &password, const std::string &salt) {
    SHA256 sha256;
    std::string combined = password + salt;
    return sha256(combined);
}

bool verify_password(const std::string &input_password, const std::string &stored_hash, const std::string &stored_salt) {
    std::string input_hash = hash_password(input_password, stored_salt);

    return input_hash == stored_hash;
}




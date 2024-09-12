//
// Created by takusi on 9/3/2024.
//

#include <iostream>
#include <sstream>
#include <random>
#include <iomanip>
#include "PasswordHasher.h"
#include "../../hash256/sha256.h"

PasswordHasher::PasswordHasher() : stored_salt(generate_salt()) {}

std::string PasswordHasher::getStoredSalt() const {
    return stored_salt;
}


std::string PasswordHasher::hashPassword(const std::string &password) const {
    return hashPassword(password, stored_salt);
}

std::string PasswordHasher::hashPassword(const std::string &password, const std::string &salt){
    SHA256 sha256;
    std::string combined = password + salt;
    return sha256(combined);
}

std::string PasswordHasher::generate_salt(const size_t length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
    }
    return ss.str();
}



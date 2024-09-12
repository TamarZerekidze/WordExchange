#pragma once
#include <string>

constexpr int SALT_LEN = 16;

/**
 * @class PasswordHasher
 * @brief Handles password hashing and salt management.
 *
 * The `PasswordHasher` class:
 * - Stores and provides a salt (`getStoredSalt`).
 * - Hashes passwords using the stored salt (`hashPassword`).
 * - Provides static methods to hash passwords with a given salt (`hashPassword`).
 * - Generates a random salt (`generate_salt`).
 */

class PasswordHasher {
public:
    PasswordHasher();

    [[nodiscard]] std::string getStoredSalt() const;

    [[nodiscard]] std::string hashPassword(const std::string &password) const;

    [[nodiscard]] static std::string hashPassword(const std::string &password, const std::string &salt) ;

private:
    std::string stored_salt;

    [[nodiscard]] static std::string generate_salt(size_t length = SALT_LEN) ;

};



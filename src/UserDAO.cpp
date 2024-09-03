#include <iostream>
#include <sqlite3.h>
#include "User.h"
#include "DatabaseConnection.h"
#include "UserDAO.h"

/* TODO: throw exceptions instead of couts*/
UserDAO::UserDAO() = default;

bool UserDAO::userExists(const std::string &username){
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sql = "SELECT COUNT(*) FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0;
}

long long UserDAO::addUser(User& user){
    if (userExists(user.getUsername())) return -1;
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sql = "INSERT INTO users (username, password_hash, time_added) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, user.getUsername().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.getPassword().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, user.getTimeAdded());

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to add user: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    const long long new_id = sqlite3_last_insert_rowid(db);
    user.setUserId(new_id);
    return new_id;
}

bool UserDAO::removeUser(const long long uid){
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sql = "DELETE FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int64(stmt, 1, uid);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to remove user: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    sqlite3_finalize(stmt);
    return true;
}
/*TODO: make unique_ptr (smart-pointers) for getUserByUsername */
[[nodiscard]] std::unique_ptr<User> UserDAO::getUserByUsername(const std::string &username){
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sql = "SELECT id, username, password_hash, time_added FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    std::unique_ptr<User> user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const long long id = sqlite3_column_int64(stmt, 0);
        const std::string uname = (const char*)(sqlite3_column_text(stmt, 1));
        const std::string password = (const char*)(sqlite3_column_text(stmt, 2));
        const time_t time_added = sqlite3_column_int64(stmt, 3);
        user = std::make_unique<User>(uname, password, time_added);
        user->setUserId(id);
    } else {
        std::cerr << "No user found with username: " << username << std::endl;
    }

    sqlite3_finalize(stmt);
    return user;
}

bool UserDAO::isValidUser(const std::string &username, const std::string &pass){
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sql = "SELECT COUNT(*) FROM users WHERE username = ? AND password_hash = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, pass.c_str(), -1, SQLITE_TRANSIENT);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0;
}



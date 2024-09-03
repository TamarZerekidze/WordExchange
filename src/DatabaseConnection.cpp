
#include "DatabaseConnection.h"
#include <iostream>


std::unique_ptr<DatabaseConnection> DatabaseConnection::instance = nullptr;
std::mutex DatabaseConnection::mtx;

DatabaseConnection::DatabaseConnection() : db(nullptr) {
    if (sqlite3_open("C:/Users/takusi/CLionProjects/WordExchange/database", &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Opened database successfully!" << std::endl;
    }
}

DatabaseConnection::~DatabaseConnection() {
    if (db) {
        sqlite3_close(db);
        std::cout << "Closed database connection." << std::endl;
    }
}
/* TODO: could return *instance as DatabaseConnection&, safer */
DatabaseConnection& DatabaseConnection::getInstance() {
    std::lock_guard lock(mtx);
    if (!instance) {
        instance.reset(new DatabaseConnection());
    }
    return *instance;
}

sqlite3* DatabaseConnection::getConnection() const {
    return db;
}


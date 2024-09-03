#pragma once

#include <sqlite3.h>
#include <mutex>
#include <memory>

class DatabaseConnection {
private:
    static std::unique_ptr<DatabaseConnection> instance;
    static std::mutex mtx;
    sqlite3* db;

    DatabaseConnection();


public:
    ~DatabaseConnection();
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    static DatabaseConnection& getInstance();
    [[nodiscard]] sqlite3* getConnection() const;
};


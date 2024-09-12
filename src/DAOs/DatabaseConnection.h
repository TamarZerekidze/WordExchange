#pragma once

#include <../sqlite/sqlite3.h>
#include <mutex>
#include <memory>

/* DatabaseConnection class uses singleton pattern to avoid creating many connection instances
 * whenever DAO class connects to the database. it creates unique pointer to the instance once dynamically
 * and uses this instance object for future calls. I used smart pointer to avoid memory leaks after the program finishes
 * as they are automatically deallocated after execution is finished. */

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


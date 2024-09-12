#pragma once

#include <../sqlite/sqlite3.h>
#include <mutex>
#include <memory>

/**
 * @class DatabaseConnection
 * @brief Implements a singleton pattern for managing a single database connection.
 *
 * The `DatabaseConnection` class uses a singleton pattern to ensure only one instance of the database connection is created.
 * It employs a unique pointer to manage the instance dynamically, ensuring that the connection is properly deallocated
 * when the program finishes. The unique pointer prevents memory leaks by automatically deleting the instance when it
 * goes out of scope or is no longer needed.
 */

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


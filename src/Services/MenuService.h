#pragma once
#include <mutex>
#include <string>

/**
 * @class MenuService
 * @brief Provides methods for fetching leaderboard and game session data.
 *
 * The `MenuService` class offers static methods to retrieve:
 * - Leaderboard information (`fetchLeaderboard`), given a username and a mutex for thread safety.
 * - Details of a specific game session (`fetchGameSession`), given a game ID and a mutex for thread safety.
 * - mutex is needed, as multiple threads access GameDAO.
 */

class MenuService {

public:
    MenuService() = default;
    ~MenuService() = default;

    static std::string fetchLeaderboard(const std::string& uname, std::mutex& gameDaoMutex);
    static std::string fetchGameSession(int gameId, std::mutex& gameDaoMutex);
};


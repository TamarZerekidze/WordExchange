#pragma once
#include <string>
#include <vector>
#include "../Objects/Session.h"

/**
 * @class GameDAO
 * @brief Manages game session and player data in the database.
 *
 * Provides static methods for:
 * - Adding sessions (`addSession`).
 * - Retrieving the best plays and session data (`getUserBestPlay`, `getUserSessions`).
 * - Getting top sessions and averages (`getTopSession`, `getUserAverage`, `getAverage`).
 * - Fetching game rounds (`getGameRounds`).
 */

class GameDAO {
public:
    GameDAO();
    static long long addSession(std::vector<std::pair<std::string, std::string> >& rounds, Session& session);
    static std::vector<std::pair<std::string, int> > getUserBestPlay(const std::string& uname);
    [[nodiscard]] static std::vector<Session> getUserSessions(const std::string& uname);
    static std::vector<Session> getTopSession();
    static double getUserAverage(const std::string& uname);
    static double getAverage();
    static std::vector<std::pair<std::string, std::string> > getGameRounds(int gameId);

};

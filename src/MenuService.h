#pragma once
#include <mutex>
#include <string>

class MenuService {

public:
    MenuService() = default;
    ~MenuService() = default;

    static std::string fetchLeaderboard(const std::string& uname, std::mutex& gameDaoMutex);
    static std::string fetchGameSession(int gameId, std::mutex& gameDaoMutex);

};


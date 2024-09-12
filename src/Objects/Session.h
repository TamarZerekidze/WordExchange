#pragma once
#include <string>
#include <chrono>
#include <ctime>

class Session {
private:
    long long id = -1;
    std::string player_1;
    std::string player_2;
    int roundNum = 0;
    std::time_t timeFinished = 0;

public:
    Session();
    Session(std::string player_1, std::string player_2, int roundNum);
    Session(std::string player_1, std::string player_2, int roundNum, std::time_t timeFinished);

    [[nodiscard]] long long getId() const ;
    [[nodiscard]] std::string getPlayer1() const ;
    [[nodiscard]] std::string getPlayer2() const;
    [[nodiscard]] int getRoundNum() const;
    [[nodiscard]] time_t getTimeFinished() const;

    void setGameId(long long id);
    void setPlayer1(const std::string &player1);
    void setPlayer2(const std::string &player2);
    void setRoundNum(int roundNum);
    void setTimeFinished(std::time_t timeFinished);

};



#include "Session.h"
using namespace std;

Session::Session() = default;
Session::Session(std::string player_1, std::string player_2, const int roundNum)
    : player_1(std::move(player_1)), player_2(std::move(player_2)), roundNum(roundNum), timeFinished(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())){}

Session::Session(std::string player_1, std::string player_2, const int roundNum, const time_t timeFinished) : player_1(std::move(player_1)), player_2(std::move(player_2)), roundNum(roundNum), timeFinished(timeFinished) {}

[[nodiscard]] long long Session::getId() const {
    return this->id;
}

[[nodiscard]] std::string Session::getPlayer1() const {
    return this->player_1;
}

[[nodiscard]] std::string Session::getPlayer2() const {
    return this->player_2;
}

[[nodiscard]] int Session::getRoundNum() const {
    return this->roundNum;
}

[[nodiscard]] time_t Session::getTimeFinished() const {
    const time_t t = this->timeFinished;
    return t;
}

void Session::setGameId(const long long id) {
    this->id = id;
}

void Session::setPlayer1(const std::string &player1) {
    this->player_1 = player1;
}

void Session::setPlayer2(const std::string &player2) {
    this->player_2 = player2;
}

void Session::setRoundNum(const int roundNum) {
    this->roundNum = roundNum;
}

void Session::setTimeFinished(const std::time_t timeFinished) {
    this->timeFinished = timeFinished;
}


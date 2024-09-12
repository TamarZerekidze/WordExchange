#include <vector>
#include "MenuService.h"
#include "../DAOs/GameDAO.h"

std::string timeToString(const std::time_t timeFinished) {
    const std::tm localTime = *std::localtime(&timeFinished);
    std::ostringstream timeStream;
    timeStream << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return timeStream.str();
}

// Construction of tables in string representation.
std::string createBoard(std::vector<std::pair<std::string, int> >& userBest, std::vector<Session>& userSessions, std::vector<Session>& topSession, double userAv, double av) {
    std::string board;
    const std::string personalBest = " The best player score obtained with partner(s): \n";
    board += personalBest;
    if(userBest.empty()) {
        board += "You have no partner(s)!\n";
    } else {
        std::ostringstream leaderboard;
        leaderboard << "+-----------------+--------+\n";
        leaderboard << "| Name            | Rounds |\n";
        leaderboard << "+-----------------+--------+\n";
        for (const auto&[fst, snd] : userBest) {
            leaderboard << "| " << std::setw(15) << std::left << fst << " | "
                        << std::setw(6) << std::right << snd << " |\n";
        }
        leaderboard << "+-----------------+--------+\n";
        board += leaderboard.str();
    }

    board += "\n Ranked gaming sessions of the player: \n";
    if (userSessions.empty()) {
        board += "You have no session(s)!\n";
    } else {
        std::ostringstream leaderboard1;
        leaderboard1 << "+------+--------+-----------------+--------+----------------------+\n";
        leaderboard1 << "| Rank | GameId | Partner         | Rounds |       Date           |\n";
        leaderboard1 << "+------+--------+-----------------+--------+----------------------+\n";
        int rank = 1;
        for (const auto& session : userSessions) {
            leaderboard1 << "| " << std::setw(4) << std::left << rank << " | "
                        << std::setw(6) << std::left << session.getId() << " | "
                        << std::setw(15) << std::left << session.getPlayer2() << " | "
                        << std::setw(6) << std::right << session.getRoundNum() << " | "
                        << std::setw(20) << std::right << timeToString(session.getTimeFinished()) << " |\n";
            rank++;
        }
        leaderboard1 << "+------+--------+-----------------+--------+----------------------+\n";
        board += leaderboard1.str();
    }

    board += "\n Average of the player: " + std::to_string(userAv) +" \n";
    board += "\n Global average: " + std::to_string(av) +" \n";

    board += "\n Top gameplay globally: \n";
    if (topSession.empty()) {
        board += "No session(s)!\n";
    } else {
        std::ostringstream leaderboard2;
        leaderboard2 << "+-----------------+-----------------+--------+----------------------+\n";
        leaderboard2 << "| Player 1        | Player 2        | Rounds |       Date           |\n";
        leaderboard2 << "+-----------------+-----------------+--------+----------------------+\n";
        for (const auto& session : topSession) {
            leaderboard2 << "| " << std::setw(15) << std::left << session.getPlayer1() << " | "
                        << std::setw(15) << std::left << session.getPlayer2() << " | "
                        << std::setw(6) << std::right << session.getRoundNum() << " | "
                        << std::setw(20) << std::right << timeToString(session.getTimeFinished()) << " |\n";
        }
        leaderboard2 << "+-----------------+-----------------+--------+----------------------+\n";
        board += leaderboard2.str();
    }
    return board;
}

std::string createSession(const std::vector<std::pair<std::string, std::string> >& gameRounds) {
    std::string session = " Game Session flow: \n";
    if(gameRounds.empty()) {
        session += "No rounds!\n";
    } else {
        std::ostringstream leaderboard;
        leaderboard << "+-----------------+-----------------+\n";
        for (const auto&[fst, snd] : gameRounds) {
            leaderboard << "| " << std::setw(15) << std::left << fst << " | "
                        << std::setw(15) << std::left << snd << " |\n";
        }
        leaderboard << "+-----------------+-----------------+\n";
        session += leaderboard.str();
    }
    return session;
}

std::string MenuService::fetchLeaderboard(const std::string& uname, std::mutex& gameDaoMutex) {
    std::string leaderboard = uname + "'s stats and leaderboard: \n";
    {
        std::lock_guard lock(gameDaoMutex);
        std::vector<std::pair<std::string, int> > userBest = GameDAO::getUserBestPlay(uname);
        std::vector<Session> userSessions = GameDAO::getUserSessions(uname);
        std::vector<Session> topSession = GameDAO::getTopSession();
        const double userAv = GameDAO::getUserAverage(uname);
        const double av = GameDAO::getAverage();
        leaderboard += createBoard(userBest, userSessions, topSession, userAv, av);
    }
    return leaderboard;
}


std::string MenuService::fetchGameSession(const int gameId, std::mutex& gameDaoMutex) {
    std::string gameSession;
    {
        std::lock_guard lock(gameDaoMutex);
        const std::vector<std::pair<std::string, std::string> > gameRounds =  GameDAO::getGameRounds(gameId);
        gameSession = createSession(gameRounds);
    }
    return gameSession;
}
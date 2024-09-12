#include <iostream>
#include <../sqlite/sqlite3.h>
#include "../Objects/Session.h"
#include "DatabaseConnection.h"
#include "GameDAO.h"
GameDAO::GameDAO() = default;

long long GameDAO::addSession(std::vector<std::pair<std::string, std::string> >& rounds, Session& session) {
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sqlInsertSession = "INSERT INTO sessions (player_1, player_2, num_rounds, time_finished) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sqlInsertSession.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
    sqlite3_bind_text(stmt, 1, session.getPlayer1().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, session.getPlayer2().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, session.getRoundNum());
    sqlite3_bind_int64(stmt, 4, session.getTimeFinished());
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to add session: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }
    sqlite3_finalize(stmt);
    const long long gameId = sqlite3_last_insert_rowid(db);

    const std::string sqlInsertWords = "INSERT INTO words (game_id,input_1, input_2) VALUES (?, ?, ?);";
    for (const auto&[fst, snd] : rounds) {
        if (sqlite3_prepare_v2(db, sqlInsertWords.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare words statement: " << sqlite3_errmsg(db) << std::endl;
            return -1;
        }
        sqlite3_bind_int64(stmt, 1, gameId);
        sqlite3_bind_text(stmt, 2, fst.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, snd.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to add word round: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return -1;
        }
        sqlite3_finalize(stmt);
    }
    return gameId;
}

std::vector<std::pair<std::string, int> > GameDAO::getUserBestPlay(const std::string& uname) {
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    // First, find the lowest roundNum for the given user
    const std::string sql_min_round =
        "SELECT MIN(num_rounds) FROM sessions WHERE player_1 = ? OR player_2 = ?;";
    sqlite3_stmt* stmt_min_round;
    if (sqlite3_prepare_v2(db, sql_min_round.c_str(), -1, &stmt_min_round, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return {};
    }
    sqlite3_bind_text(stmt_min_round, 1, uname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt_min_round, 2, uname.c_str(), -1, SQLITE_TRANSIENT);
    int min_round = 0;
    if (sqlite3_step(stmt_min_round) == SQLITE_ROW) {
        min_round = sqlite3_column_int(stmt_min_round, 0);
    }
    sqlite3_finalize(stmt_min_round);
    if (min_round == 0) {
        //std::cerr << "No sessions found for username: " << uname << std::endl;
        return {};
    }

    // Now retrieve all sessions where num_rounds equals the minimum found
    const std::string sql =
        "SELECT player_1, player_2, num_rounds FROM sessions "
        "WHERE (player_1 = ? OR player_2 = ?) AND num_rounds = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return {};
    }
    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, uname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, min_round);

    std::vector<std::pair<std::string, int>> result;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const std::string player1 = (const char*)sqlite3_column_text(stmt, 0);
        const std::string player2 = (const char*)sqlite3_column_text(stmt, 1);
        const int rounds = sqlite3_column_int(stmt, 2);
        if (rounds == min_round) {
            if (player1 == uname) {
                result.emplace_back(player2, rounds);
            } else {
                result.emplace_back(player1, rounds);
            }
        }
    }
    sqlite3_finalize(stmt);
    return result;
}

[[nodiscard]] std::vector<Session> GameDAO::getUserSessions(const std::string& uname) {
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sql = "SELECT game_id, player_1, player_2, num_rounds, time_finished FROM sessions WHERE player_1 = ? OR player_2 = ? ORDER BY num_rounds ASC LIMIT 10;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return {};
    }
    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, uname.c_str(), -1, SQLITE_TRANSIENT);

    std::vector<Session> sessions;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const long long gameId = sqlite3_column_int64(stmt, 0);
        const std::string player1 = (const char*)sqlite3_column_text(stmt, 1);
        const std::string player2 = (const char*)sqlite3_column_text(stmt, 2);
        const int rounds = sqlite3_column_int(stmt, 3);
        const time_t timeFinished = sqlite3_column_int64(stmt, 4);
        Session session;
        if (player1 == uname) { session = Session(player1, player2, rounds, timeFinished);}
        else { session = Session(player2, player1, rounds, timeFinished);}
        session.setGameId(gameId);
        sessions.emplace_back(session);
    }
    sqlite3_finalize(stmt);
    return sessions;
}

std::vector<Session> GameDAO::getTopSession() {
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    // First, find the lowest roundNum
    const std::string sql_min_round =
        "SELECT MIN(num_rounds) FROM sessions;";
    sqlite3_stmt* stmt_min_round;
    if (sqlite3_prepare_v2(db, sql_min_round.c_str(), -1, &stmt_min_round, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return {};
    }
    int min_round = 0;
    if (sqlite3_step(stmt_min_round) == SQLITE_ROW) {
        min_round = sqlite3_column_int(stmt_min_round, 0);
    }
    sqlite3_finalize(stmt_min_round);

    // Now retrieve all sessions where num_rounds equals the minimum found
    const std::string sql =
        "SELECT game_id, player_1, player_2, num_rounds, time_finished FROM sessions "
        "WHERE num_rounds = ? ORDER BY time_finished DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return {};
    }
    sqlite3_bind_int(stmt, 1, min_round);

    std::vector<Session> sessions;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const long long gameId = sqlite3_column_int64(stmt, 0);
        const std::string player1 = (const char*)sqlite3_column_text(stmt, 1);
        const std::string player2 = (const char*)sqlite3_column_text(stmt, 2);
        const int rounds = sqlite3_column_int(stmt, 3);
        const time_t timeFinished = sqlite3_column_int64(stmt, 4);
        Session session;
        session = Session(player1, player2, rounds, timeFinished);
        session.setGameId(gameId);
        sessions.emplace_back(session);
        }
    sqlite3_finalize(stmt);
    return sessions;
}

double GameDAO::getUserAverage(const std::string& uname) {
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sql = "SELECT AVG(num_rounds) FROM sessions WHERE player_1 = ? OR player_2 = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return 0.0;
    }
    sqlite3_bind_text(stmt, 1, uname.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, uname.c_str(), -1, SQLITE_TRANSIENT);
    double average = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        average = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return average;
}

double GameDAO::getAverage() {
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    const std::string sql = "SELECT AVG(num_rounds) FROM sessions;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return 0.0;
    }
    double average = 0.0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        average = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return average;
}

std::vector<std::pair<std::string, std::string> > GameDAO::getGameRounds(const int gameId) {
    sqlite3* db = DatabaseConnection::getInstance().getConnection();

    std::vector<std::pair<std::string, std::string>> rounds;
    const std::string pairSql = "SELECT player_1, player_2 FROM sessions WHERE game_id = ?;";
    sqlite3_stmt* pairStmt;
    if (sqlite3_prepare_v2(db, pairSql.c_str(), -1, &pairStmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return {};
    }
    sqlite3_bind_int(pairStmt, 1, gameId);
    if(sqlite3_step(pairStmt) == SQLITE_ROW) {
        const std::string player1 = (const char*)sqlite3_column_text(pairStmt, 0);
        const std::string player2 = (const char*)sqlite3_column_text(pairStmt, 1);
        rounds.emplace_back(player1, player2);
    }
    sqlite3_finalize(pairStmt);

    const std::string sql = "SELECT input_1, input_2 FROM words WHERE game_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return {};
    }
    sqlite3_bind_int(stmt, 1, gameId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const std::string input1 = (const char*)sqlite3_column_text(stmt, 0);
        const std::string input2 = (const char*)sqlite3_column_text(stmt, 1);
        rounds.emplace_back(input1, input2);
    }
    sqlite3_finalize(stmt);
    return rounds;
}
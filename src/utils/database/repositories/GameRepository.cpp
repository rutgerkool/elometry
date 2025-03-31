#include "utils/database/repositories/GameRepository.h"
#include <iostream>

GameRepository::GameRepository(Database& database)
    : m_db(database.getConnection()) {
}

std::vector<Game> GameRepository::fetchGames() const {
    return executeQuery(BASE_QUERY);
}

std::optional<Game> GameRepository::fetchGameById(int gameId) const {
    std::string query = std::string(BASE_QUERY) + " WHERE cg.game_id = ?";
    auto games = executeQuery(query, gameId);
    
    return games.empty() ? std::nullopt : std::optional{games.front()};
}

std::vector<Game> GameRepository::fetchGamesForClub(int clubId) const {
    std::string query = std::string(BASE_QUERY) + " WHERE cg.club_id = ? OR cg.opponent_id = ?";
    return executeQuery(query, clubId, clubId);
}

std::vector<Game> GameRepository::fetchRecentGames(int limit) const {
    std::string query = std::string(BASE_QUERY) + " ORDER BY g.date DESC LIMIT ?";
    return executeQuery(query, limit);
}

template<typename... Args>
std::vector<Game> GameRepository::executeQuery(const std::string& query, Args... args) const {
    sqlite3_stmt* stmt = nullptr;
    std::vector<Game> games;
    
    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(m_db) << std::endl;
        return games;
    }
    
    if constexpr (sizeof...(args) > 0) {
        int paramIndex = 1;
        (sqlite3_bind_int(stmt, paramIndex++, args), ...);
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        games.push_back(extractGameFromStatement(stmt));
    }
    
    sqlite3_finalize(stmt);
    return games;
}

Game GameRepository::extractGameFromStatement(sqlite3_stmt* stmt) const {
    Game game;
    game.gameId = sqlite3_column_int(stmt, 0);
    game.homeClubId = sqlite3_column_int(stmt, 1);
    game.homeGoals = sqlite3_column_int(stmt, 2);
    game.awayClubId = sqlite3_column_int(stmt, 3);
    game.awayGoals = sqlite3_column_int(stmt, 4);
    
    const unsigned char* homeClubName = sqlite3_column_text(stmt, 5);
    const unsigned char* awayClubName = sqlite3_column_text(stmt, 6);
    const unsigned char* dateText = sqlite3_column_text(stmt, 7);
    
    game.homeClubName = homeClubName ? reinterpret_cast<const char*>(homeClubName) : "Unknown";
    game.awayClubName = awayClubName ? reinterpret_cast<const char*>(awayClubName) : "Unknown";
    game.date = dateText ? reinterpret_cast<const char*>(dateText) : "";
    
    return game;
}

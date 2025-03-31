#include "utils/database/repositories/AppearanceRepository.h"
#include <array>
#include <cassert>
#include <algorithm>
#include <ranges>
#include <string>
#include <utility>

AppearanceRepository::AppearanceRepository(Database& database) 
    : m_db(database.getConnection()) {
    assert(m_db != nullptr);
}

std::vector<PlayerAppearance> AppearanceRepository::fetchAppearances() const {
    return executeQuery(BASE_QUERY);
}

std::vector<PlayerAppearance> AppearanceRepository::fetchPlayerAppearances(PlayerId playerId) const {
    std::string query = std::string(BASE_QUERY) + " WHERE player_id = ?";
    return executeQuery(query, playerId);
}

std::vector<PlayerAppearance> AppearanceRepository::fetchGameAppearances(int gameId) const {
    std::string query = std::string(BASE_QUERY) + " WHERE game_id = ?";
    return executeQuery(query, gameId);
}

std::optional<PlayerAppearance> AppearanceRepository::fetchAppearance(PlayerId playerId, int gameId) const {
    std::string query = std::string(BASE_QUERY) + " WHERE player_id = ? AND game_id = ?";
    
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    
    sqlite3_bind_int(stmt, 1, playerId);
    sqlite3_bind_int(stmt, 2, gameId);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        PlayerAppearance appearance = extractAppearanceFromStatement(stmt);
        sqlite3_finalize(stmt);
        return appearance;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

template<typename... Args>
std::vector<PlayerAppearance> AppearanceRepository::executeQuery(const std::string& query, Args... args) const {
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return {};
    }
    
    if constexpr (sizeof...(args) > 0) {
        int paramIndex = 1;
        (sqlite3_bind_int(stmt, paramIndex++, args), ...);
    }
    
    return bindAndExecute(stmt);
}

std::vector<PlayerAppearance> AppearanceRepository::bindAndExecute(sqlite3_stmt* stmt) const {
    std::vector<PlayerAppearance> appearances;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        appearances.push_back(extractAppearanceFromStatement(stmt));
    }
    
    sqlite3_finalize(stmt);
    return appearances;
}

PlayerAppearance AppearanceRepository::extractAppearanceFromStatement(sqlite3_stmt* stmt) const {
    return PlayerAppearance{
        .playerId = sqlite3_column_int(stmt, 1),
        .clubId = sqlite3_column_int(stmt, 2),
        .gameId = sqlite3_column_int(stmt, 0),
        .goals = sqlite3_column_int(stmt, 3),
        .assists = sqlite3_column_int(stmt, 4),
        .minutesPlayed = sqlite3_column_int(stmt, 5)
    };
}

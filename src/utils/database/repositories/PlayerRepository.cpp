#include "utils/database/repositories/PlayerRepository.h"
#include "utils/database/PlayerMapper.h"
#include "utils/Season.h"
#include <iostream>
#include <array>
#include <algorithm>
#include <ranges>
#include <format>

PlayerRepository::PlayerRepository(Database& database)
    : m_db(database.getConnection()) {
}

PlayerRepository::PlayerRepository(sqlite3* db)
    : m_db(db) {
}

std::vector<Player> PlayerRepository::fetchPlayers(std::optional<int> clubId,
                                                  std::optional<int> playerId,
                                                  std::optional<int> teamId) const {
    int currentSeasonYear = getCurrentSeasonYear();

    std::string query = R"(
        SELECT 
            player_id, 
            current_club_id, 
            name, 
            current_club_name, 
            sub_position, 
            position, 
            contract_expiration_date, 
            market_value_in_eur, 
            highest_market_value_in_eur,
            image_url
        FROM players
        WHERE last_season = ?
    )";

    std::vector<std::pair<int, int>> params = {{1, currentSeasonYear}};
    int paramIndex = 2;

    if (clubId && *clubId != -1) {
        query += " AND current_club_id = ?";
        params.emplace_back(paramIndex++, *clubId);
    }

    if (playerId && *playerId != -1) {
        query += " AND player_id = ?";
        params.emplace_back(paramIndex++, *playerId);
    }

    if (teamId && *teamId != -1) {
        query += " AND player_id IN (SELECT player_id FROM team_players WHERE team_id = ?)";
        params.emplace_back(paramIndex, *teamId);
    }

    return executeQuery(query, params);
}

std::optional<Player> PlayerRepository::fetchPlayerById(int playerId) const {
    auto players = fetchPlayers(std::nullopt, playerId);
    return players.empty() ? std::nullopt : std::optional{players.front()};
}

std::vector<Player> PlayerRepository::fetchPlayersByClub(int clubId) const {
    return fetchPlayers(clubId);
}

std::vector<Player> PlayerRepository::fetchPlayersByTeam(int teamId) const {
    return fetchPlayers(std::nullopt, std::nullopt, teamId);
}

std::vector<Player> PlayerRepository::executeQuery(std::string_view query, 
                                                  std::span<const std::pair<int, int>> params) const {
    std::vector<Player> players;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, query.data(), static_cast<int>(query.size()), &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL error: " << sqlite3_errmsg(m_db) << std::endl;
        return players;
    }

    for (const auto& [index, value] : params) {
        sqlite3_bind_int(stmt, index, value);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        players.push_back(PlayerMapper::mapPlayerFromStatement(stmt));
    }

    sqlite3_finalize(stmt);
    return players;
}

#include "utils/database/repositories/TeamRepository.h"
#include "utils/database/PlayerMapper.h"
#include "utils/Season.h"
#include <iostream>

TeamRepository::TeamRepository(Database& database) {
    db = database.getConnection();
}

std::vector<std::string> TeamRepository::getAvailableSubPositions() {
    std::vector<std::string> subPositions;
    sqlite3_stmt *stmt;
    std::string query = "SELECT DISTINCT sub_position FROM players WHERE sub_position IS NOT NULL;";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            subPositions.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
    }

    sqlite3_finalize(stmt);
    return subPositions;
}

std::vector<Player> TeamRepository::fetchPlayersForClub(int clubId) {
    std::vector<Player> players;
    sqlite3_stmt *stmt;

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
            highest_market_value_in_eur 
        FROM players 
        WHERE current_club_id = ? AND last_season = ?;
    )";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, clubId);
        sqlite3_bind_int(stmt, 2, currentSeasonYear);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Player player = PlayerMapper::mapPlayerFromStatement(stmt);
            players.push_back(player);
        }
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return players;
}

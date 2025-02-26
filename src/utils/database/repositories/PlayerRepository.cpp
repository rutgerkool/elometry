#include "utils/database/repositories/PlayerRepository.h"
#include "utils/database/PlayerMapper.h"
#include "utils/Season.h"
#include <iostream>

PlayerRepository::PlayerRepository(Database& database) {
    db = database.getConnection();
}

PlayerRepository::PlayerRepository(sqlite3 * db): db(db) {}

std::vector<Player> PlayerRepository::fetchPlayers(int clubId, int playerId) {
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
            highest_market_value_in_eur,
            image_url
        FROM players
        WHERE last_season = ?
    )";

    if (clubId != -1) { 
        query += " AND current_club_id = ?";
    }

    if (playerId != -1) { 
        query += " AND player_id = ?";
    }

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, currentSeasonYear);

        int paramIndex = 2;
        if (clubId != -1) {
            sqlite3_bind_int(stmt, paramIndex++, clubId);
        }
        if (playerId != -1) {
            sqlite3_bind_int(stmt, paramIndex, playerId);
        }

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

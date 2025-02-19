#include "utils/database/repositories/PlayerRepository.h"
#include <iostream>

PlayerRepository::PlayerRepository(Database& database) {
    db = database.getConnection();
}

PlayerRepository::PlayerRepository(sqlite3 * db): db(db) {}

Player PlayerRepository::getPlayerFromStatement(sqlite3_stmt *stmt) {
    Player player;
    player.playerId = sqlite3_column_int(stmt, 0);

    const unsigned char* name = sqlite3_column_text(stmt, 1);
    player.name = name ? std::string(reinterpret_cast<const char*>(name)) : "";

    player.clubId = sqlite3_column_int(stmt, 2);

    const unsigned char* clubName = sqlite3_column_text(stmt, 3);
    player.clubName = clubName ? std::string(reinterpret_cast<const char*>(clubName)) : "";

    const unsigned char* subPosition = sqlite3_column_text(stmt, 4);
    player.subPosition = subPosition ? std::string(reinterpret_cast<const char*>(subPosition)) : "";

    const unsigned char* position = sqlite3_column_text(stmt, 5);
    player.position = position ? std::string(reinterpret_cast<const char*>(position)) : "";

    const unsigned char* contractExpirationDate = sqlite3_column_text(stmt, 6);
    player.contractExpirationDate = contractExpirationDate ? std::string(reinterpret_cast<const char*>(contractExpirationDate)) : "";

    player.marketValue = sqlite3_column_int(stmt, 7);
    player.highestMarketValue = sqlite3_column_int(stmt, 8);

    return player;
}

std::vector<Player> PlayerRepository::fetchPlayers(const int& playerId) {
    std::vector<Player> players;
    sqlite3_stmt *stmt;
    std::string query = R"(
        SELECT 
            player_id, 
            name, 
            current_club_id, 
            current_club_name, 
            sub_position, 
            position, 
            contract_expiration_date, 
            market_value_in_eur, 
            highest_market_value_in_eur 
        FROM players
    )";

    if (playerId != -1) { 
        query += " WHERE player_id = ?";
    }

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        if (playerId != -1) {
            sqlite3_bind_int(stmt, 1, playerId);
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Player player = getPlayerFromStatement(stmt);

            players.push_back(player);
        }
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return players;
}

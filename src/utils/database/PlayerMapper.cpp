#include "utils/database/PlayerMapper.h"

Player PlayerMapper::mapPlayerFromStatement(sqlite3_stmt *stmt) {
    Player player;
    player.playerId = sqlite3_column_int(stmt, 0);
    player.clubId = sqlite3_column_int(stmt, 1);

    const unsigned char* name = sqlite3_column_text(stmt, 2);
    player.name = name ? std::string(reinterpret_cast<const char*>(name)) : "";

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

    const unsigned char* imageUrl = sqlite3_column_text(stmt, 9);
    player.imageUrl = imageUrl ? std::string(reinterpret_cast<const char*>(imageUrl)) : "";

    return player;
}

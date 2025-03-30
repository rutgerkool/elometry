#include "utils/database/PlayerMapper.h"
#include <string>

Player PlayerMapper::mapPlayerFromStatement(sqlite3_stmt* stmt) {
    Player player;
    player.playerId = extractIntColumn(stmt, 0);
    player.clubId = extractIntColumn(stmt, 1);
    player.name = extractTextColumn(stmt, 2);
    player.clubName = extractTextColumn(stmt, 3);
    player.subPosition = extractTextColumn(stmt, 4);
    player.position = extractTextColumn(stmt, 5);
    player.contractExpirationDate = extractTextColumn(stmt, 6);
    player.marketValue = extractIntColumn(stmt, 7);
    player.highestMarketValue = extractIntColumn(stmt, 8);
    player.imageUrl = extractTextColumn(stmt, 9);
    
    return player;
}

std::optional<Player> PlayerMapper::tryMapPlayerFromStatement(sqlite3_stmt* stmt) {
    if (!stmt) {
        return std::nullopt;
    }
    
    try {
        return mapPlayerFromStatement(stmt);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

std::string PlayerMapper::extractTextColumn(sqlite3_stmt* stmt, int columnIndex) {
    if (isColumnNull(stmt, columnIndex)) {
        return {};
    }
    
    const unsigned char* text = sqlite3_column_text(stmt, columnIndex);
    return text ? std::string(reinterpret_cast<const char*>(text)) : std::string{};
}

int PlayerMapper::extractIntColumn(sqlite3_stmt* stmt, int columnIndex) {
    if (isColumnNull(stmt, columnIndex)) {
        return 0;
    }
    
    return sqlite3_column_int(stmt, columnIndex);
}

bool PlayerMapper::isColumnNull(sqlite3_stmt* stmt, int columnIndex) {
    return sqlite3_column_type(stmt, columnIndex) == SQLITE_NULL;
}

#ifndef PLAYERMAPPER_H
#define PLAYERMAPPER_H

#include "models/PlayerRating.h"
#include <sqlite3.h>
#include <string_view>
#include <optional>

class PlayerMapper {
public:
    static Player mapPlayerFromStatement(sqlite3_stmt* stmt);
    
    static std::optional<Player> tryMapPlayerFromStatement(sqlite3_stmt* stmt);
    
private:
    static std::string extractTextColumn(sqlite3_stmt* stmt, int columnIndex);
    
    static int extractIntColumn(sqlite3_stmt* stmt, int columnIndex);
    
    static bool isColumnNull(sqlite3_stmt* stmt, int columnIndex);
};

#endif

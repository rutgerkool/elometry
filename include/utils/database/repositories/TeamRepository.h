#ifndef TEAMREPOSITORY_H
#define TEAMREPOSITORY_H

#include "utils/database/Database.h"
#include "models/PlayerRating.h"
#include <vector>

class TeamRepository {
public:
    explicit TeamRepository(Database& database);

    std::vector<std::string> getAvailableSubPositions();

    std::vector<Player> fetchPlayersForClub(int clubId);

private:
    sqlite3* db;
    Player getPlayerFromStatement(sqlite3_stmt *stmt);
};

#endif 

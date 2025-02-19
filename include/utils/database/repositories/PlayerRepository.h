#ifndef PLAYERREPOSITORY_H
#define PLAYERREPOSITORY_H

#include "utils/database/Database.h"
#include <vector>

struct Player {
    PlayerId playerId;
    std::string name;
    int clubId;
    std::string clubName;
    std::string subPosition;
    std::string position;
    std::string contractExpirationDate;
    int marketValue;
    int highestMarketValue;
};

class PlayerRepository {
    public:
        PlayerRepository(Database& database);
        std::vector<Player> fetchPlayers(const int& playerId = -1);

    private:
        sqlite3 * db;
};

#endif

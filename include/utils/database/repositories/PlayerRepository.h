#ifndef PLAYERREPOSITORY_H
#define PLAYERREPOSITORY_H

#include "utils/database/Database.h"
#include <vector>

struct Player {
    PlayerId playerId;
    int clubId;
    std::string name;
    std::string clubName;
    std::string subPosition;
    std::string position;
    std::string contractExpirationDate;
    int marketValue;
    int highestMarketValue;

    double rating = 1500.00;
    int minutesPlayed = 0;
};

class PlayerRepository {
    public:
        PlayerRepository(Database& database);
        PlayerRepository(sqlite3 * db);
        std::vector<Player> fetchPlayers(const int& playerId = -1);

    private:
        sqlite3 * db;
};

#endif

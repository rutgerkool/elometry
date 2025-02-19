#ifndef APPEARANCEREPOSITORY_H
#define APPEARANCEREPOSITORY_H

#include "utils/database/Database.h"
#include <vector>

struct PlayerAppearance {
    PlayerId playerId;
    int clubId;
    int gameId;
    int goals;
    int assists;
    int minutesPlayed;
};

class AppearanceRepository {
    public:
        AppearanceRepository(Database& database);
        std::vector<PlayerAppearance> fetchAppearances();

    private:
        sqlite3 * db;
};

#endif

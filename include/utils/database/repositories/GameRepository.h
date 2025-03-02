#ifndef GAMEREPOSITORY_H
#define GAMEREPOSITORY_H

#include "utils/database/Database.h"
#include <vector>
#include <string>

struct Game {
    int gameId;
    int homeClubId;
    int awayClubId;
    int homeGoals;
    int awayGoals;
    std::string homeClubName;
    std::string awayClubName;
    std::string date;
};

class GameRepository {
    public:
        GameRepository(Database& db);
        std::vector<Game> fetchGames();

    private:
        sqlite3 * db;
};

#endif

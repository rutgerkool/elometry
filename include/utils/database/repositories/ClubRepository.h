#ifndef CLUBREPOSITORY_H
#define CLUBREPOSITORY_H

#include "utils/database/Database.h"
#include <vector>

struct Club {
    int clubId;
    std::string clubName;
};

class ClubRepository {
    public:
        ClubRepository(Database& database);
        std::vector<Club> fetchClubs(const int& clubId = -1);

    private:
        sqlite3 * db;
};

#endif

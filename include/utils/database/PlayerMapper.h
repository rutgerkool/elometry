#ifndef PLAYERMAPPER_H
#define PLAYERMAPPER_H

#include "models/PlayerRating.h"
#include <sqlite3.h>

class PlayerMapper {
    public:
        static Player mapPlayerFromStatement(sqlite3_stmt *stmt);
};

#endif 

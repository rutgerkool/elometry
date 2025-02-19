#ifndef RATINGMANAGER_H
#define RATINGMANAGER_H

#include "models/PlayerRating.h"

class RatingManager {
private:
    Database& database;
    PlayerRating ratingSystem;

public:
    RatingManager(Database& db);

    void loadAndProcessRatings();
};

#endif

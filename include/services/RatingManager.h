#ifndef RATINGMANAGER_H
#define RATINGMANAGER_H

#include "models/PlayerRating.h"
#include "models/ILPSelector.h"

class RatingManager {
private:
    Database& database;
    PlayerRating ratingSystem;

public:
    RatingManager(Database& db);

    void loadAndProcessRatings();

    std::vector<std::pair<int, Player>> selectOptimalTeamByPositions(
        const std::vector<std::string>& requiredPositions,
        int budget
    );
};

#endif

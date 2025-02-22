#include "services/RatingManager.h"
#include <iostream>

int main() {
    Database database("test.db");
    RatingManager ratingManager(database);

    ratingManager.loadAndProcessRatings();

    ratingManager.selectOptimalTeamByPositions({"Attacking Midfield", "Centre-Back"}, 50000000);

    return 0;
}

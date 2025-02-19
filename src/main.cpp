#include "services/RatingManager.h"
#include <iostream>

int main() {
    Database database("test.db");
    RatingManager ratingManager(database);

    ratingManager.loadAndProcessRatings();

    return 0;
}

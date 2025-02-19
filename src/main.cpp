#include "models/PlayerRating.h"
#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"
#include <iostream>

int main() {
    Database database("test.db");
    GameRepository gameRepository(database);
    AppearanceRepository appearanceRepository(database);

    std::vector<Game> games = gameRepository.fetchGames();
    std::vector<PlayerAppearance>appearances = appearanceRepository.fetchAppearances();

    PlayerRating ratingSystem;

    ratingSystem.processMatchesParallel(games, appearances);

    ratingSystem.saveRatingsToFile();
    return 0;
}

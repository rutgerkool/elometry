#include "services/RatingManager.h"
#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"

RatingManager::RatingManager(Database& db): database(db) {}

void RatingManager::loadAndProcessRatings() {
    GameRepository gameRepository(database);
    AppearanceRepository appearanceRepository(database);

    std::vector<Game> games = gameRepository.fetchGames();
    std::vector<PlayerAppearance>appearances = appearanceRepository.fetchAppearances();

    ratingSystem.processMatchesParallel(games, appearances);
    ratingSystem.saveRatingsToFile();
}

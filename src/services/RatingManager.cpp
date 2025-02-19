#include "services/RatingManager.h"
#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"
#include "utils/database/repositories/PlayerRepository.h"

RatingManager::RatingManager(Database& db): database(db) {}

void RatingManager::loadAndProcessRatings() {
    GameRepository gameRepository(database);
    AppearanceRepository appearanceRepository(database);
    PlayerRepository playerRepository(database);

    std::vector<Game> games = gameRepository.fetchGames();
    std::vector<PlayerAppearance> appearances = appearanceRepository.fetchAppearances();
    std::vector<Player> players = playerRepository.fetchPlayers();

    for (const auto& player : players) {
        ratingSystem.initializePlayer(player);
    }

    ratingSystem.processMatchesParallel(games, appearances);
    ratingSystem.saveRatingsToFile();
}

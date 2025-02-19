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

    for (const auto& game : games) {
        std::vector<PlayerAppearance> matchAppearances;
        
        for (const auto& appearance : appearances) {
            if (appearance.gameId == game.gameId) {
                ratingSystem.initializePlayer(appearance.playerId);
                matchAppearances.push_back(appearance);
            }
        } 

        ratingSystem.processMatch(game, matchAppearances);
    }

    ratingSystem.saveRatingsToFile();
    return 0;
}

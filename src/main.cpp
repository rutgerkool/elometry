#include "models/PlayerRating.h"
#include "utils/DataLoader.h"
#include <iostream>

int main() {
    PlayerRating ratingSystem;
    DataLoader loader;

    std::vector<Game> games = loader.loadGames("../data/club_games.csv");
    std::vector<PlayerAppearance>appearances = loader.loadAppearances("../data/appearances.csv");
    std::cout << "Data loaded\n";

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

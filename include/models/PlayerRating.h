#ifndef PLAYERRATING_H
#define PLAYERRATING_H

#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"
#include "utils/database/repositories/PlayerRepository.h"
#include <unordered_map>
#include <vector>
#include <string>

class PlayerRating {
    public:
        PlayerRating(double k = 20.0, double homeAdvantage = 100.0);

        void initializePlayer(Player player);
        double calculateExpectation(double playerRating, double opponentTeamRating);
        double updateRating(double currentRating, double expected, double actual, int minutesPlayed, int goalDifference);
        void processMatch(const Game& game, const std::vector<PlayerAppearance>& appearances);
        static bool sortPlayersByRating(const std::pair<int, Player>& a, const std::pair<int, Player>& b);
        void processMatchesParallel(const std::vector<Game>& games, const std::vector<PlayerAppearance>& appearances);
        void saveRatingsToFile();
        std::vector<std::pair<int, Player>>  getSortedRatedPlayers();

    private:
        std::unordered_map<PlayerId, Player> ratedPlayers;
        double kFactor;
        double homeAdvantage;
};

#endif

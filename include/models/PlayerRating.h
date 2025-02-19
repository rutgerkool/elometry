#ifndef PLAYERRATING_H
#define PLAYERRATING_H

#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"
#include <unordered_map>
#include <vector>
#include <string>

class PlayerRating {
    public:
        PlayerRating(double k = 20.0, double homeAdvantage = 100.0);

        void initializePlayer(PlayerId playerId, double initialRating = 1500.0);
        double calculateExpectation(double playerRating, double opponentTeamRating);
        double updateRating(double currentRating, double expected, double actual, int minutesPlayed, int goalDifference);
        void processMatch(const Game& game, const std::vector<PlayerAppearance>& appearances);
        void saveRatingsToFile();

    private:
        std::unordered_map<PlayerId, double> playerRatings;
        std::unordered_map<PlayerId, int> playerMinutes;
        double kFactor;
        double homeAdvantage;
};

#endif

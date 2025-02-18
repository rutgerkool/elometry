#include "models/PlayerRating.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

PlayerRating::PlayerRating(double k, double homeAdvantage) 
    : kFactor(k), homeAdvantage(homeAdvantage) {}

void PlayerRating::initializePlayer(PlayerId playerId, double initialRating) {
    if (playerRatings.find(playerId) == playerRatings.end()) {
        playerRatings[playerId] = initialRating;
        playerMinutes[playerId] = 0;
    }
}

double PlayerRating::calculateExpectation(double playerRating, double opponentTeamRating) {
    return 1.0 / (1.0 + pow(10.0, (opponentTeamRating - playerRating) / 400.0));
}

double PlayerRating::updateRating(double currentRating, double expected, double actual, int minutesPlayed, int goalDifference) {
    double performanceFactor = (static_cast<double>(minutesPlayed) / 90.0) * (1.0 + static_cast<double>(goalDifference) / 5.0);
    return currentRating + kFactor * performanceFactor * (actual - expected);
}

void PlayerRating::processMatch(const Game& game, const std::vector<PlayerAppearance>& appearances) {
    double homeTeamRating = 0.0, awayTeamRating = 0.0;
    int homeCount = 0, awayCount = 0;

    for (const auto& player : appearances) {
        if (player.clubId == game.homeClubId) {
            homeTeamRating += playerRatings[player.playerId];
            homeCount++;
        } else {
            awayTeamRating += playerRatings[player.playerId];
            awayCount++;
        }
    }

    if (homeCount > 0)
        homeTeamRating /= homeCount;
    if (awayCount > 0)
        awayTeamRating /= awayCount;

    double homeExpected = calculateExpectation(homeTeamRating + homeAdvantage, awayTeamRating);
    double awayExpected = calculateExpectation(awayTeamRating, homeTeamRating + homeAdvantage);

    double homeActual = game.homeGoals > game.awayGoals ? 1.0 : (game.homeGoals == game.awayGoals ? 0.5 : 0.0);
    double awayActual = 1.0 - homeActual;

    for (const auto& player : appearances) {
        double expected = player.clubId == game.homeClubId ? homeExpected : awayExpected;
        double actual = player.clubId == game.homeClubId ? homeActual : awayActual;

        playerRatings[player.playerId] = updateRating(playerRatings[player.playerId], expected, actual, player.minutesPlayed, std::abs(game.homeGoals - game.awayGoals));
        playerMinutes[player.playerId] += player.minutesPlayed;
    }
}

void PlayerRating::saveRatingsToFile() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream filename;
    filename << "player_ratings_" << std::put_time(std::localtime(&nowTime), "%Y%m%d_%H%M%S") << ".csv";

    std::ofstream outFile(filename.str());

    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for saving ratings." << std::endl;
    }

    outFile << "PlayerId,Rating,MinutesPlayed\n";

    for (const auto& [playerId, rating] : playerRatings) {
        outFile << playerId << "," << rating << "," << playerMinutes[playerId] << "\n";
    }

    outFile.close();
    std::cout << "Player ratings saved to player_ratings.csv" << std::endl;
}

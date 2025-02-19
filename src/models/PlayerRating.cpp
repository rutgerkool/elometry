#include "models/PlayerRating.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

PlayerRating::PlayerRating(double k, double homeAdvantage) 
    : kFactor(k), homeAdvantage(homeAdvantage) {}

void PlayerRating::initializePlayer(Player player) {
    if (ratedPlayers.find(player.playerId) == ratedPlayers.end()) {
        ratedPlayers[player.playerId] = player;
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
            homeTeamRating += ratedPlayers[player.playerId].rating;
            homeCount++;
        } else {
            awayTeamRating += ratedPlayers[player.playerId].rating;
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

        double newRating = updateRating(ratedPlayers[player.playerId].rating, expected, actual, player.minutesPlayed, std::abs(game.homeGoals - game.awayGoals));

        #pragma omp critical
        {
            ratedPlayers[player.playerId].rating = newRating;
            ratedPlayers[player.playerId].minutesPlayed += player.minutesPlayed;
        }
    }
}


void PlayerRating::processMatchesParallel(const std::vector<Game>& games, const std::vector<PlayerAppearance>& appearances) {
    std::unordered_map<int, std::vector<PlayerAppearance>> gameAppearances;

    for (const auto& appearance : appearances) {
        gameAppearances[appearance.gameId].push_back(appearance);
    }

    std::vector<Game> sortedGames = games;
    std::sort(sortedGames.begin(), sortedGames.end(), [](const Game& a, const Game& b) {
        return a.gameId < b.gameId;
    });

    #pragma omp parallel for ordered
    for (size_t i = 0; i < sortedGames.size(); ++i) {
        int gameId = sortedGames[i].gameId;

        if (gameAppearances.find(gameId) != gameAppearances.end()) {
            #pragma omp ordered
            processMatch(sortedGames[i], gameAppearances[gameId]);
        }
    }
}

bool PlayerRating::sortPlayersByRating(const std::pair<int, Player>& a, const std::pair<int, Player>& b) {
    return a.second.rating > b.second.rating;
}

void PlayerRating::saveRatingsToFile() {
    auto now = std::chrono::system_clock::now();
    std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream filename;
    filename << "player_ratings_" << std::put_time(std::localtime(&nowTime), "%Y%m%d_%H%M%S") << ".csv";

    std::ofstream outFile(filename.str());

    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for saving ratings." << std::endl;
        return;
    }

    outFile << "PlayerId,Name,Rating,SubPosition,Position,MarketValue,HighestMarketValue\n";

    std::vector<std::pair<int, Player>> sortedRatings(ratedPlayers.begin(), ratedPlayers.end());
    std::sort(sortedRatings.begin(), sortedRatings.end(), sortPlayersByRating);

    for (const auto& [playerId, rating] : sortedRatings) {
        outFile 
            << playerId << "," 
            << ratedPlayers[playerId].name << "," 
            << ratedPlayers[playerId].rating << "," 
            << ratedPlayers[playerId].subPosition << ","
            << ratedPlayers[playerId].position << ","
            << ratedPlayers[playerId].marketValue << ","
            << ratedPlayers[playerId].highestMarketValue << "\n";
    }

    outFile.close();
    std::cout << "Player ratings saved to " << filename.str() << std::endl;
}

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
        ratingHistory[player.playerId] = std::deque<RatingChange>();
    }
}

double PlayerRating::calculateExpectation(double playerRating, double opponentTeamRating) {
    return 1.0 / (1.0 + pow(10.0, (opponentTeamRating - playerRating) / 400.0));
}

double PlayerRating::updateRating(double currentRating, double expected, double actual, int minutesPlayed, int goalDifference) {
    double performanceFactor = (static_cast<double>(minutesPlayed) / 90.0) * (1.0 + static_cast<double>(goalDifference) / 5.0);
    return currentRating + kFactor * performanceFactor * (actual - expected);
}

void PlayerRating::calculateTeamRatings(
    const Game& game, const std::vector<PlayerAppearance>& appearances,
    double& homeTeamRating, 
    double& awayTeamRating
) {
    int homeCount = 0, awayCount = 0;
    homeTeamRating = 0.0;
    awayTeamRating = 0.0;

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
}

void PlayerRating::calculateMatchExpectations(double homeTeamRating, double awayTeamRating, double& homeExpected, double& awayExpected) {
    homeExpected = calculateExpectation(homeTeamRating + homeAdvantage, awayTeamRating);
    awayExpected = calculateExpectation(awayTeamRating, homeTeamRating + homeAdvantage);
}

double PlayerRating::calculateActualResult(int homeGoals, int awayGoals) {
    if (homeGoals > awayGoals) return 1.0;
    if (homeGoals == awayGoals) return 0.5;
    return 0.0;
}

double PlayerRating::calculateMatchImpact(double kFactor, int minutesPlayed, int goalDifference, double actual, double expected) {
    return kFactor * (static_cast<double>(minutesPlayed) / 90.0) * 
           (1.0 + static_cast<double>(goalDifference) / 5.0) * 
           (actual - expected);
}

void PlayerRating::createRatingChangeRecord(
    const PlayerAppearance& player,
    const Game& game,
    double previousRating,
    double newRating,
    double expected,
    double actual
) {
    bool isHomeGame = player.clubId == game.homeClubId;
    int goalDifference = isHomeGame ? (game.homeGoals - game.awayGoals) : (game.awayGoals - game.homeGoals);
    std::string opponent = isHomeGame ? game.awayClubName : game.homeClubName;
    
    double matchImpact = calculateMatchImpact(kFactor, player.minutesPlayed, std::abs(goalDifference), actual, expected);

    RatingChange change;
    change.gameId = game.gameId;
    change.previousRating = previousRating;
    change.newRating = newRating;
    change.opponent = opponent;
    change.isHomeGame = isHomeGame;
    change.minutesPlayed = player.minutesPlayed;
    change.goalDifference = goalDifference;
    change.matchImpact = matchImpact;
    change.goals = player.goals;
    change.assists = player.assists;
    change.date = game.date;
    
    ratingHistory[player.playerId].push_back(change);
    
    if (ratingHistory[player.playerId].size() > MAX_HISTORY_SIZE) {
        ratingHistory[player.playerId].pop_front();
    }
}

void PlayerRating::updatePlayerRating(const PlayerAppearance& player, const Game& game, double expected, double actual) {
    double previousRating = ratedPlayers[player.playerId].rating;
    bool isHomeGame = player.clubId == game.homeClubId;
    int goalDifference = isHomeGame ? (game.homeGoals - game.awayGoals) : (game.awayGoals - game.homeGoals);
    
    double newRating = updateRating(previousRating, expected, actual, player.minutesPlayed, std::abs(goalDifference));
    
    #pragma omp critical
    {
        ratedPlayers[player.playerId].rating = newRating;
        ratedPlayers[player.playerId].minutesPlayed += player.minutesPlayed;
        
        createRatingChangeRecord(player, game, previousRating, newRating, expected, actual);
    }
}

void PlayerRating::processMatch(const Game& game, const std::vector<PlayerAppearance>& appearances) {
    double homeTeamRating, awayTeamRating;
    calculateTeamRatings(game, appearances, homeTeamRating, awayTeamRating);
    
    double homeExpected, awayExpected;
    calculateMatchExpectations(homeTeamRating, awayTeamRating, homeExpected, awayExpected);
    
    double homeActual = calculateActualResult(game.homeGoals, game.awayGoals);
    double awayActual = 1.0 - homeActual;

    for (const auto& player : appearances) {
        double expected = player.clubId == game.homeClubId ? homeExpected : awayExpected;
        double actual = player.clubId == game.homeClubId ? homeActual : awayActual;
        
        updatePlayerRating(player, game, expected, actual);
    }
}

void PlayerRating::groupAppearancesByGame(const std::vector<PlayerAppearance>& appearances,
                                      std::unordered_map<int, std::vector<PlayerAppearance>>& gameAppearances) {
    for (const auto& appearance : appearances) {
        if (ratedPlayers.find(appearance.playerId) != ratedPlayers.end()) {
            gameAppearances[appearance.gameId].push_back(appearance);
        }
    }
}

void PlayerRating::sortGamesByDate(const std::vector<Game>& games, std::vector<Game>& sortedGames) {
    sortedGames = games;
    std::sort(sortedGames.begin(), sortedGames.end(), [](const Game& a, const Game& b) {
        return a.date < b.date;
    });
}

void PlayerRating::processMatchesParallel(const std::vector<Game>& games, const std::vector<PlayerAppearance>& appearances) {
    std::unordered_map<int, std::vector<PlayerAppearance>> gameAppearances;
    groupAppearancesByGame(appearances, gameAppearances);

    std::vector<Game> sortedGames;
    sortGamesByDate(games, sortedGames);

    #pragma omp parallel for ordered
    for (int i = 0; i < sortedGames.size(); ++i) {
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

std::vector<std::pair<int, Player>> PlayerRating::getSortedRatedPlayers() {
    std::vector<std::pair<int, Player>> sortedRatedPlayers(ratedPlayers.begin(), ratedPlayers.end());
    std::sort(sortedRatedPlayers.begin(), sortedRatedPlayers.end(), sortPlayersByRating);
    return sortedRatedPlayers;
}

std::vector<RatingChange> PlayerRating::getPlayerRatingHistory(int playerId, int maxGames) {
    std::vector<RatingChange> history;
    
    auto it = ratingHistory.find(playerId);
    if (it != ratingHistory.end()) {
        const auto& playerHistory = it->second;
        int numEntries = std::min(static_cast<int>(playerHistory.size()), maxGames);
        
        history.reserve(numEntries);
        for (int i = 0; i < numEntries; ++i) {
            history.push_back(playerHistory[i]);
        }
    }
    
    return history;
}

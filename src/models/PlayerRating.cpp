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
    
    ratedPlayers[player.playerId].rating = newRating;
    ratedPlayers[player.playerId].minutesPlayed += player.minutesPlayed;
    
    createRatingChangeRecord(player, game, previousRating, newRating, expected, actual);
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

void PlayerRating::prepareGameCalculations(
    const std::vector<Game>& sortedGames, 
    const std::unordered_map<int, std::vector<PlayerAppearance>>& gameAppearances,
    std::vector<GameCalculations>& calculations
) {
    calculations.resize(sortedGames.size());
    
    #pragma omp parallel for
    for (size_t i = 0; i < sortedGames.size(); ++i) {
        const Game& game = sortedGames[i];
        int gameId = game.gameId;
        
        auto it = gameAppearances.find(gameId);
        if (it == gameAppearances.end()) {
            calculations[i].gameId = 0;
            continue;
        }
        
        GameCalculations& calc = calculations[i];
        calc.gameId = gameId;
        calc.game = &game;
        calc.playerAppearances = it->second;
        
        calculateTeamRatings(game, calc.playerAppearances, calc.homeTeamRating, calc.awayTeamRating);
        calculateMatchExpectations(calc.homeTeamRating, calc.awayTeamRating, calc.homeExpected, calc.awayExpected);
        calc.homeActual = calculateActualResult(game.homeGoals, game.awayGoals);
    }
}

void PlayerRating::applyRatingChanges(const std::vector<GameCalculations>& calculations) {
    for (const auto& calc : calculations) {
        if (calc.gameId == 0) continue;
        
        const Game& game = *calc.game;
        double awayActual = 1.0 - calc.homeActual;
        
        for (const auto& player : calc.playerAppearances) {
            double expected = player.clubId == game.homeClubId ? calc.homeExpected : calc.awayExpected;
            double actual = player.clubId == game.homeClubId ? calc.homeActual : awayActual;
            
            updatePlayerRating(player, game, expected, actual);
        }
    }
}

void PlayerRating::processMatchesParallel(const std::vector<Game>& games, const std::vector<PlayerAppearance>& appearances) {
    std::unordered_map<int, std::vector<PlayerAppearance>> gameAppearances;
    groupAppearancesByGame(appearances, gameAppearances);

    std::vector<Game> sortedGames;
    sortGamesByDate(games, sortedGames);
    
    for (size_t i = 0; i < sortedGames.size(); ++i) {
        int gameId = sortedGames[i].gameId;
        
        auto it = gameAppearances.find(gameId);
        if (it != gameAppearances.end()) {
            processMatch(sortedGames[i], it->second);
        }
    }
}

bool PlayerRating::sortPlayersByRating(const std::pair<int, Player>& a, const std::pair<int, Player>& b) {
    return a.second.rating > b.second.rating;
}

std::vector<std::pair<int, Player>> PlayerRating::getSortedRatedPlayers() const {
    std::vector<std::pair<int, Player>> sortedRatedPlayers(ratedPlayers.begin(), ratedPlayers.end());
    std::sort(sortedRatedPlayers.begin(), sortedRatedPlayers.end(), sortPlayersByRating);
    return sortedRatedPlayers;
}

std::vector<RatingChange> PlayerRating::getPlayerRatingHistory(int playerId, int maxGames) const{
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

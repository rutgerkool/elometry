#include "models/PlayerRating.h"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <numeric>
#include <execution>

PlayerRating::PlayerRating(double k, double homeAdvantage)
    : kFactor(k), homeAdvantage(homeAdvantage) {}

void PlayerRating::initializePlayer(const Player& player) {
    if (ratedPlayers.find(player.playerId) == ratedPlayers.end()) {
        ratedPlayers[player.playerId] = player;
        ratingHistory[player.playerId] = std::deque<RatingChange>();
    }
}

double PlayerRating::calculateExpectation(double playerRating, double opponentTeamRating) const {
    return 1.0 / (1.0 + std::pow(10.0, (opponentTeamRating - playerRating) / 400.0));
}

double PlayerRating::updateRating(double currentRating, double expected, double actual, int minutesPlayed, int goalDifference) const {
    double performanceFactor = (static_cast<double>(minutesPlayed) / 90.0) * 
                               (1.0 + static_cast<double>(goalDifference) / 5.0);
    return currentRating + kFactor * performanceFactor * (actual - expected);
}

double PlayerRating::calculateMatchImpact(int minutesPlayed, int goalDifference, double actual, double expected) const {
    return kFactor * (static_cast<double>(minutesPlayed) / 90.0) * 
           (1.0 + static_cast<double>(goalDifference) / 5.0) * (actual - expected);
}

double PlayerRating::calculateActualResult(int homeGoals, int awayGoals) {
    if (homeGoals > awayGoals) return 1.0;
    if (homeGoals == awayGoals) return 0.5;
    return 0.0;
}

void PlayerRating::calculateTeamRatings(
    const Game& game, 
    std::span<const PlayerAppearance> appearances,
    double& homeTeamRating, 
    double& awayTeamRating) const 
{
    int homeCount = 0, awayCount = 0;
    homeTeamRating = 0.0;
    awayTeamRating = 0.0;

    for (const auto& player : appearances) {
        if (ratedPlayers.find(player.playerId) == ratedPlayers.end()) {
            continue;
        }

        if (player.clubId == game.homeClubId) {
            homeTeamRating += ratedPlayers.at(player.playerId).rating;
            homeCount++;
        } else if (player.clubId == game.awayClubId) {
            awayTeamRating += ratedPlayers.at(player.playerId).rating;
            awayCount++;
        }
    }

    if (homeCount > 0) {
        homeTeamRating /= homeCount;
    }
    if (awayCount > 0) {
        awayTeamRating /= awayCount;
    }
}

void PlayerRating::calculateMatchExpectations(
    double homeTeamRating, 
    double awayTeamRating, 
    double& homeExpected, 
    double& awayExpected) const 
{
    homeExpected = calculateExpectation(homeTeamRating + homeAdvantage, awayTeamRating);
    awayExpected = calculateExpectation(awayTeamRating, homeTeamRating + homeAdvantage);
}

void PlayerRating::createRatingChangeRecord(
    const PlayerAppearance& player,
    const Game& game,
    double previousRating,
    double newRating,
    double expected,
    double actual)
{
    bool isHomeGame = player.clubId == game.homeClubId;
    int goalDifference = isHomeGame ? (game.homeGoals - game.awayGoals) : (game.awayGoals - game.homeGoals);
    std::string opponent = isHomeGame ? game.awayClubName : game.homeClubName;
    
    double matchImpact = calculateMatchImpact(player.minutesPlayed, std::abs(goalDifference), actual, expected);

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
    
    ratingHistory[player.playerId].push_front(change);
    
    if (ratingHistory[player.playerId].size() > MAX_HISTORY_SIZE) {
        ratingHistory[player.playerId].pop_back();
    }
}

void PlayerRating::updatePlayerRating(
    const PlayerAppearance& player, 
    const Game& game, 
    double expected, 
    double actual)
{
    if (ratedPlayers.find(player.playerId) == ratedPlayers.end()) {
        return;
    }

    bool isHomeGame = player.clubId == game.homeClubId;
    int goalDifference = isHomeGame ? 
        (game.homeGoals - game.awayGoals) : 
        (game.awayGoals - game.homeGoals);
    
    double previousRating = ratedPlayers[player.playerId].rating;
    double newRating = updateRating(
        previousRating, 
        expected, 
        actual, 
        player.minutesPlayed,
        std::abs(goalDifference)
    );
    
    ratedPlayers[player.playerId].rating = newRating;
    ratedPlayers[player.playerId].minutesPlayed += player.minutesPlayed;
    
    createRatingChangeRecord(player, game, previousRating, newRating, expected, actual);
}

void PlayerRating::processMatch(const Game& game, std::span<const PlayerAppearance> appearances) {
    double homeTeamRating = 0.0, awayTeamRating = 0.0;
    calculateTeamRatings(game, appearances, homeTeamRating, awayTeamRating);
    
    double homeExpected = 0.0, awayExpected = 0.0;
    calculateMatchExpectations(homeTeamRating, awayTeamRating, homeExpected, awayExpected);
    
    double homeActual = calculateActualResult(game.homeGoals, game.awayGoals);
    double awayActual = 1.0 - homeActual;

    for (const auto& player : appearances) {
        if (ratedPlayers.find(player.playerId) == ratedPlayers.end()) {
            continue;
        }
        
        double expected = (player.clubId == game.homeClubId) ? homeExpected : awayExpected;
        double actual = (player.clubId == game.homeClubId) ? homeActual : awayActual;
        
        updatePlayerRating(player, game, expected, actual);
    }
}

std::unordered_map<int, std::vector<PlayerAppearance>> PlayerRating::groupAppearancesByGame(
    std::span<const PlayerAppearance> appearances) const 
{
    std::unordered_map<int, std::vector<PlayerAppearance>> gameAppearances;
    
    for (const auto& appearance : appearances) {
        if (ratedPlayers.find(appearance.playerId) != ratedPlayers.end()) {
            gameAppearances[appearance.gameId].push_back(appearance);
        }
    }
    
    return gameAppearances;
}

std::vector<Game> PlayerRating::sortGamesByDate(std::span<const Game> games) const {
    std::vector<Game> sortedGames(games.begin(), games.end());
    
    std::sort(sortedGames.begin(), sortedGames.end(), 
        [](const Game& a, const Game& b) { return a.date < b.date; });
    
    return sortedGames;
}

std::vector<GameCalculations> PlayerRating::prepareGameCalculations(
    std::span<const Game> sortedGames,
    const std::unordered_map<int, std::vector<PlayerAppearance>>& gameAppearances) const 
{
    std::vector<GameCalculations> calculations(sortedGames.size());
    
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
    
    return calculations;
}

void PlayerRating::applyRatingChanges(std::span<const GameCalculations> calculations) {
    for (const auto& calc : calculations) {
        if (calc.gameId == 0 || calc.game == nullptr) {
            continue;
        }
        
        const Game& game = *calc.game;
        double awayActual = 1.0 - calc.homeActual;
        
        for (const auto& player : calc.playerAppearances) {
            if (ratedPlayers.find(player.playerId) == ratedPlayers.end()) {
                continue;
            }
            
            double expected = (player.clubId == game.homeClubId) ? calc.homeExpected : calc.awayExpected;
            double actual = (player.clubId == game.homeClubId) ? calc.homeActual : awayActual;
            
            updatePlayerRating(player, game, expected, actual);
        }
    }
}

void PlayerRating::processMatches(
    std::span<const Game> games, 
    std::span<const PlayerAppearance> appearances)
{
    auto gameAppearances = groupAppearancesByGame(appearances);
    auto sortedGames = sortGamesByDate(games);
    
    for (const auto& game : sortedGames) {
        auto it = gameAppearances.find(game.gameId);
        if (it != gameAppearances.end()) {
            processMatch(game, it->second);
        }
    }
}

void PlayerRating::processMatchesParallel(
    const std::vector<Game>& games, 
    const std::vector<PlayerAppearance>& appearances)
{
    auto gameAppearances = groupAppearancesByGame(appearances);
    auto sortedGames = sortGamesByDate(games);
    
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

std::vector<RatingChange> PlayerRating::getPlayerRatingHistory(
    int playerId, 
    int maxGames) const
{
    std::vector<RatingChange> history;
    
    auto it = ratingHistory.find(playerId);
    if (it == ratingHistory.end()) {
        return history;
    }
    
    const auto& playerHistory = it->second;
    int numEntries = std::min(static_cast<int>(playerHistory.size()), maxGames);
    
    history.reserve(numEntries);
    std::copy_n(playerHistory.begin(), numEntries, std::back_inserter(history));
    
    return history;
}

std::vector<std::pair<int, Player>> PlayerRating::getSortedRatedPlayers() const {
    std::vector<std::pair<int, Player>> sortedPlayers(ratedPlayers.begin(), ratedPlayers.end());
    std::sort(sortedPlayers.begin(), sortedPlayers.end(), sortPlayersByRating);
    return sortedPlayers;
}

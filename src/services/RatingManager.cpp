#include "services/RatingManager.h"
#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"
#include "utils/database/repositories/PlayerRepository.h"
#include <algorithm>
#include <unordered_set>
#include <iostream>
#include <iomanip>

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
}

std::vector<std::pair<int, Player>> RatingManager::selectOptimalTeamByPositions(
    const std::vector<std::string>& requiredPositions,
    int64_t budget
) {
    std::vector<std::pair<int, Player>> sortedRatedPlayers = ratingSystem.getSortedRatedPlayers();
    ILPSelector selector(sortedRatedPlayers, requiredPositions, budget);
    return selector.selectTeam();
}

std::vector<Player> RatingManager::getFilteredRatedPlayers(const std::vector<Player>& filterPlayers) {
    std::vector<std::pair<int, Player>> allRatedPlayers = ratingSystem.getSortedRatedPlayers();
    
    std::unordered_set<int> filterPlayerIds;
    for (const auto& player : filterPlayers) {
        filterPlayerIds.insert(player.playerId);
    }
    
    std::vector<Player> filteredPlayers;
    
    for (const auto& ratedPlayer : allRatedPlayers) {
        if (filterPlayerIds.count(ratedPlayer.second.playerId) > 0) {
            filteredPlayers.push_back(ratedPlayer.second);
        }
    }
    
    return filteredPlayers;
}

std::vector<std::pair<int, double>> RatingManager::getRecentRatingProgression(int playerId, int maxGames) {
    std::vector<RatingChange> history = ratingSystem.getPlayerRatingHistory(playerId, maxGames);
    std::vector<std::pair<int, double>> progression;
    
    if (history.empty()) {
        return progression;
    }
    
    progression.reserve(history.size());
    
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        progression.push_back(std::make_pair(it->gameId, it->newRating));
    }
    
    return progression;
}

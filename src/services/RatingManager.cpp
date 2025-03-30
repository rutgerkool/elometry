#include "services/RatingManager.h"
#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"
#include "utils/database/repositories/PlayerRepository.h"
#include "utils/database/Database.h"
#include "models/ILPSelector.h"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <execution>

RatingManager::RatingManager(Database& database)
    : m_database(database)
    , m_ratingSystem(std::make_unique<PlayerRating>())
    , m_gameRepository(createGameRepository())
    , m_appearanceRepository(createAppearanceRepository())
    , m_playerRepository(createPlayerRepository())
{
}

std::unique_ptr<GameRepository> RatingManager::createGameRepository() const {
    return std::make_unique<GameRepository>(m_database);
}

std::unique_ptr<AppearanceRepository> RatingManager::createAppearanceRepository() const {
    return std::make_unique<AppearanceRepository>(m_database);
}

std::unique_ptr<PlayerRepository> RatingManager::createPlayerRepository() const {
    return std::make_unique<PlayerRepository>(m_database);
}

void RatingManager::loadAndProcessRatings() {
    initializePlayerRatings();
    processMatchData();
}

void RatingManager::initializePlayerRatings() {
    auto players = m_playerRepository->fetchPlayers();
    
    for (const auto& player : players) {
        m_ratingSystem->initializePlayer(player);
    }
}

void RatingManager::processMatchData() {
    auto games = m_gameRepository->fetchGames();
    auto appearances = m_appearanceRepository->fetchAppearances();
    
    m_ratingSystem->processMatchesParallel(games, appearances);
}

std::vector<std::pair<int, Player>> RatingManager::selectOptimalTeamByPositions(
    std::span<const std::string> requiredPositions,
    int64_t budget) const 
{
    auto sortedRatedPlayers = m_ratingSystem->getSortedRatedPlayers();
    ILPSelector selector(sortedRatedPlayers, requiredPositions, budget);
    
    return selector.selectTeam();
}

std::vector<Player> RatingManager::getFilteredRatedPlayers(
    std::span<const Player> filterPlayers) const 
{
    auto allRatedPlayers = m_ratingSystem->getSortedRatedPlayers();
    
    std::unordered_set<int> filterPlayerIds;
    for (const auto& player : filterPlayers) {
        filterPlayerIds.insert(player.playerId);
    }
    
    std::vector<Player> filteredPlayers;
    filteredPlayers.reserve(filterPlayerIds.size());
    
    for (const auto& [id, player] : allRatedPlayers) {
        if (filterPlayerIds.contains(player.playerId)) {
            filteredPlayers.push_back(player);
        }
    }
    
    return filteredPlayers;
}

std::vector<std::pair<int, double>> RatingManager::getRecentRatingProgression(
    int playerId, 
    int maxGames) const 
{
    auto history = m_ratingSystem->getPlayerRatingHistory(playerId, maxGames);
    
    if (history.empty()) {
        return {};
    }
    
    std::vector<std::pair<int, double>> progression;
    progression.reserve(history.size());
    
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        progression.emplace_back(it->gameId, it->newRating);
    }
    
    return progression;
}

std::vector<Player> RatingManager::getAllPlayers() const {
    auto ratedPairs = getSortedRatedPlayers();
    
    std::vector<Player> result;
    result.reserve(ratedPairs.size());
    
    std::ranges::transform(
        ratedPairs, 
        std::back_inserter(result),
        [](const auto& pair) { return pair.second; }
    );
    
    return result;
}

std::vector<RatingChange> RatingManager::getPlayerRatingHistory(
    int playerId, 
    int maxGames) const 
{
    return m_ratingSystem->getPlayerRatingHistory(playerId, maxGames);
}

std::vector<std::pair<int, Player>> RatingManager::getSortedRatedPlayers() const {
    return m_ratingSystem->getSortedRatedPlayers();
}

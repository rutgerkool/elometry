#ifndef RATINGMANAGER_H
#define RATINGMANAGER_H

#include "models/PlayerRating.h"
#include <vector>
#include <memory>
#include <span>
#include <optional>
#include <utility>
#include <cstdint>

class Database;
class TeamRepository;
class PlayerRepository;
class GameRepository;
class AppearanceRepository;

class RatingManager {
public:
    explicit RatingManager(Database& database);
    ~RatingManager() = default;

    RatingManager(const RatingManager&) = delete;
    RatingManager& operator=(const RatingManager&) = delete;
    RatingManager(RatingManager&&) = delete;
    RatingManager& operator=(RatingManager&&) = delete;

    void loadAndProcessRatings();
    
    [[nodiscard]] std::vector<std::pair<int, Player>> 
    selectOptimalTeamByPositions(
        std::span<const std::string> requiredPositions,
        int64_t budget) const;
    
    [[nodiscard]] std::vector<Player> 
    getFilteredRatedPlayers(std::span<const Player> filterPlayers) const;
    
    [[nodiscard]] std::vector<std::pair<int, double>> 
    getRecentRatingProgression(int playerId, int maxGames = 10) const;
    
    [[nodiscard]] std::vector<Player> getAllPlayers() const;
    
    [[nodiscard]] std::vector<RatingChange> 
    getPlayerRatingHistory(int playerId, int maxGames = 10) const;
    
    [[nodiscard]] std::vector<std::pair<int, Player>> 
    getSortedRatedPlayers() const;

private:
    Database& m_database;
    std::unique_ptr<PlayerRating> m_ratingSystem;
    std::unique_ptr<GameRepository> m_gameRepository;
    std::unique_ptr<AppearanceRepository> m_appearanceRepository;
    std::unique_ptr<PlayerRepository> m_playerRepository;
    
    [[nodiscard]] std::unique_ptr<GameRepository> createGameRepository() const;
    [[nodiscard]] std::unique_ptr<AppearanceRepository> createAppearanceRepository() const;
    [[nodiscard]] std::unique_ptr<PlayerRepository> createPlayerRepository() const;
    
    void initializePlayerRatings();
    void processMatchData();
};

#endif

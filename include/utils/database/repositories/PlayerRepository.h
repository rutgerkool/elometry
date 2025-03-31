#ifndef PLAYERREPOSITORY_H
#define PLAYERREPOSITORY_H

#include "utils/database/Database.h"
#include <vector>
#include <optional>
#include <string_view>
#include <span>

struct Player {
    int playerId;
    int clubId;
    std::string name;
    std::string clubName;
    std::string subPosition;
    std::string position;
    std::string contractExpirationDate;
    std::string imageUrl;
    int marketValue = 0;
    int highestMarketValue = 0;

    double rating = 1500.00;
    int minutesPlayed = 0;
};

class PlayerRepository {
public:
    explicit PlayerRepository(Database& database);
    explicit PlayerRepository(sqlite3* db);
    
    PlayerRepository(const PlayerRepository&) = delete;
    PlayerRepository& operator=(const PlayerRepository&) = delete;
    PlayerRepository(PlayerRepository&&) noexcept = default;
    PlayerRepository& operator=(PlayerRepository&&) noexcept = default;
    ~PlayerRepository() = default;
    
    [[nodiscard]] std::vector<Player> fetchPlayers(std::optional<int> clubId = std::nullopt,
                                                   std::optional<int> playerId = std::nullopt,
                                                   std::optional<int> teamId = std::nullopt) const;

    [[nodiscard]] std::optional<Player> fetchPlayerById(int playerId) const;
    [[nodiscard]] std::vector<Player> fetchPlayersByClub(int clubId) const;
    [[nodiscard]] std::vector<Player> fetchPlayersByTeam(int teamId) const;

private:
    sqlite3* m_db;

    [[nodiscard]] std::vector<Player> executeQuery(std::string_view query, 
                                                   std::span<const std::pair<int, int>> params = {}) const;
};

#endif

#ifndef APPEARANCE_REPOSITORY_H
#define APPEARANCE_REPOSITORY_H

#include "utils/database/Database.h"
#include <vector>
#include <memory>
#include <optional>
#include <span>

struct PlayerAppearance {
    PlayerId playerId;
    int clubId;
    int gameId;
    int goals;
    int assists;
    int minutesPlayed;
};

class AppearanceRepository {
public:
    explicit AppearanceRepository(Database& database);
    ~AppearanceRepository() = default;
    
    AppearanceRepository(const AppearanceRepository&) = delete;
    AppearanceRepository& operator=(const AppearanceRepository&) = delete;
    AppearanceRepository(AppearanceRepository&&) noexcept = default;
    AppearanceRepository& operator=(AppearanceRepository&&) noexcept = default;
    
    [[nodiscard]] std::vector<PlayerAppearance> fetchAppearances() const;
    [[nodiscard]] std::vector<PlayerAppearance> fetchPlayerAppearances(PlayerId playerId) const;
    [[nodiscard]] std::vector<PlayerAppearance> fetchGameAppearances(int gameId) const;
    [[nodiscard]] std::optional<PlayerAppearance> fetchAppearance(PlayerId playerId, int gameId) const;

private:
    static constexpr auto BASE_QUERY = "SELECT game_id, player_id, player_club_id, goals, assists, minutes_played FROM appearances";
    
    sqlite3* m_db;
    
    template<typename... Args>
    [[nodiscard]] std::vector<PlayerAppearance> executeQuery(const std::string& query, Args... args) const;
    
    [[nodiscard]] std::vector<PlayerAppearance> bindAndExecute(sqlite3_stmt* stmt) const;
    [[nodiscard]] PlayerAppearance extractAppearanceFromStatement(sqlite3_stmt* stmt) const;
};

#endif

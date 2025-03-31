#ifndef TEAMREPOSITORY_H
#define TEAMREPOSITORY_H

#include "utils/database/Database.h"
#include "models/PlayerRating.h"
#include "gui/models/LineupTypes.h"
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <span>

struct Team {
    int teamId;
    std::string teamName;
    std::vector<Player> players;
    int64_t budget = 20000000;
    
    Team() = default;
    Team(int id, std::string name) : teamId(id), teamName(std::move(name)) {}
    
    Team(const Team&) = default;
    Team& operator=(const Team&) = default;
    Team(Team&&) noexcept = default;
    Team& operator=(Team&&) noexcept = default;
};

class TeamRepository {
public:
    explicit TeamRepository(Database& database);
    
    [[nodiscard]] std::vector<std::string> getAvailableSubPositions() const;
    
    void createTeam(const Team& team);
    [[nodiscard]] std::vector<Team> getAllTeams() const;
    void addPlayerToTeam(int teamId, int playerId);
    void removePlayerFromTeam(int teamId, int playerId);
    void removeAllPlayersFromTeam(int teamId);
    void deleteTeam(int teamId);
    [[nodiscard]] bool updateTeamName(int teamId, std::string_view newName);
    
    [[nodiscard]] std::vector<Formation> getAllFormations() const;
    [[nodiscard]] int createLineup(int teamId, int formationId, std::string_view lineupName = "");
    [[nodiscard]] Lineup getActiveLineup(int teamId) const;
    [[nodiscard]] bool setActiveLineup(int teamId, int lineupId);
    [[nodiscard]] bool updatePlayerPosition(int lineupId, int playerId, PositionType positionType, 
                                           std::string_view fieldPosition = "", int order = 0);
    [[nodiscard]] bool saveLineup(const Lineup& lineup);
    [[nodiscard]] bool deleteLineup(int lineupId);
    [[nodiscard]] std::vector<Lineup> getTeamLineups(int teamId) const;
    void removePlayerFromAllLineups(int teamId, int playerId);
    
private:
    [[nodiscard]] std::string positionTypeToString(PositionType positionType) const;
    [[nodiscard]] PositionType stringToPositionType(std::string_view positionType) const;
    
    [[nodiscard]] bool executeStatement(std::string_view query, 
                                       std::span<const std::pair<int, int>> intBindings = {}, 
                                       std::span<const std::pair<int, std::string>> textBindings = {}) const;
    [[nodiscard]] bool executeStatement(sqlite3_stmt* stmt, 
                                       std::span<const std::pair<int, int>> intBindings = {}, 
                                       std::span<const std::pair<int, std::string>> textBindings = {}) const;
    [[nodiscard]] bool prepareStatement(std::string_view query, sqlite3_stmt** stmt) const;
    [[nodiscard]] bool deactivateTeamLineups(int teamId) const;
    [[nodiscard]] bool fillLineupPlayerPositions(Lineup& lineup) const;
    [[nodiscard]] bool saveLineupPlayers(const Lineup& lineup) const;
    
    sqlite3* m_db;
};

#endif

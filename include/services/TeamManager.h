#ifndef TEAMMANAGER_H
#define TEAMMANAGER_H

#include "utils/database/repositories/TeamRepository.h"
#include "services/RatingManager.h"
#include "utils/database/repositories/PlayerRepository.h"
#include "gui/models/LineupTypes.h"

#include <vector>
#include <string>
#include <span>
#include <optional>
#include <unordered_map>
#include <string_view>
#include <cstdint>

class TeamManager {
public:
    TeamManager(TeamRepository& teamRepo, RatingManager& ratingManager, PlayerRepository& playerRepo);
    
    [[nodiscard]] std::vector<std::string> getAvailableSubPositions() const;
    [[nodiscard]] Team& loadTeamFromClub(int clubId);
    [[nodiscard]] Team& loadTeam(int teamId);
    [[nodiscard]] Team& createTeam(std::string_view teamName);
    
    bool addPlayerToTeam(int teamId, const Player& player);
    bool removePlayerFromTeam(int teamId, int playerId);
    bool deleteTeam(int teamId);
    bool updateTeamName(int teamId, std::string_view newName);
    
    [[nodiscard]] std::vector<std::string> getMissingPositions(const Team& team) const;
    void autoFillTeam(Team& team, int64_t budget);
    void setTeamBudget(int teamId, int64_t newBudget);
    
    [[nodiscard]] std::vector<Team> getAllTeams() const;
    [[nodiscard]] Player searchPlayerById(int playerId) const;
    [[nodiscard]] std::vector<std::pair<int, std::string>> getAllClubs() const;

    void saveTeam(const Team& team);
    void loadTeams();
    void saveTeamPlayers(const Team& team);

    [[nodiscard]] RatingManager& getRatingManager() const noexcept { return m_ratingManager; }

    [[nodiscard]] std::vector<Formation> getAllFormations() const;
    [[nodiscard]] Lineup createLineup(int teamId, int formationId, std::string_view lineupName = "");
    [[nodiscard]] Lineup getActiveLineup(int teamId) const;
    bool setActiveLineup(int teamId, int lineupId);
    bool updatePlayerPosition(int lineupId, int playerId, PositionType positionType, std::string_view fieldPosition = "", int order = 0);
    bool saveLineup(const Lineup& lineup);
    bool deleteLineup(int lineupId);
    [[nodiscard]] std::vector<Lineup> getTeamLineups(int teamId) const;

private:
    TeamRepository& m_teamRepo;
    RatingManager& m_ratingManager;
    PlayerRepository& m_playerRepo;
    std::unordered_map<int, Team> m_teams;
    int m_nextTeamId = 0;

    [[nodiscard]] std::unordered_map<std::string, int> buildRequiredPositionsMap(const std::vector<std::string>& availableSubPositions) const;
    [[nodiscard]] std::unordered_map<std::string, bool> mapTeamPositions(const Team& team, const std::unordered_map<std::string, int>& requiredPositions) const;
    [[nodiscard]] std::vector<std::string> extractMissingPositions(const std::unordered_map<std::string, bool>& positionMap) const;
};

#endif

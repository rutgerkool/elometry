#ifndef TEAMMANAGER_H
#define TEAMMANAGER_H

#include "utils/database/repositories/TeamRepository.h"
#include "services/RatingManager.h"
#include "utils/database/repositories/PlayerRepository.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <algorithm>

struct Team {
    int teamId;
    std::string teamName;
    std::vector<Player> players;
    int64_t budget = 20000000;
};

class TeamManager {
public:
    TeamManager(TeamRepository& teamRepo, RatingManager& ratingManager, PlayerRepository& playerRepo);

    std::vector<std::string> getAvailableSubPositions();
    Team& loadTeamFromClub(int clubId);
    Team& loadTeam(int teamId);
    Team& createTeam(const std::string& teamName);
    
    bool addPlayerToTeam(int teamId, const Player& player);
    bool removePlayerFromTeam(int teamId, int playerId);
    bool deleteTeam(int teamId);
    
    std::vector<std::string> getMissingPositions(const Team& team);
    void autoFillTeam(Team& team, int64_t budget);
    void setTeamBudget(int teamId, int64_t newBudget);
    
    std::vector<Team> getAllTeams();
    Player searchPlayerById(int playerId);

private:
    TeamRepository& teamRepo;
    RatingManager& ratingManager;
    PlayerRepository& playerRepo;
    std::unordered_map<int, Team> teams;
    int nextTeamId = 1;
};

#endif

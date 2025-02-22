#ifndef TEAMMANAGER_H
#define TEAMMANAGER_H

#include "utils/database/repositories/TeamRepository.h"
#include "services/RatingManager.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <bits/stdc++.h>

struct Team {
    int teamId;
    std::string teamName;
    std::vector<Player> players;
};

class TeamManager {
public:
    TeamManager(TeamRepository& teamRepo, RatingManager& ratingManager);

    std::vector<std::string> getAvailableSubPositions();

    Team& loadTeamFromClub(int clubId);

    Team& createTeam(const std::string& teamName);

    bool addPlayerToTeam(int teamId, const Player& player);
    bool removePlayerFromTeam(int teamId, int playerId);

    std::vector<std::string> getMissingPositions(const Team& team);

    void autoFillTeam(Team& team, int budget);

    std::vector<Team> getAllTeams();

private:
    TeamRepository& teamRepo;
    RatingManager& ratingManager;
    std::unordered_map<int, Team> teams;
    int nextTeamId = 1;
};

#endif 

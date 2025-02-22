#include "services/TeamManager.h"
#include <iostream>

TeamManager::TeamManager(TeamRepository& teamRepo, RatingManager& ratingManager)
    : teamRepo(teamRepo), ratingManager(ratingManager) {}

std::vector<std::string> TeamManager::getAvailableSubPositions() {
    return teamRepo.getAvailableSubPositions();
}

int TeamManager::createTeam(const std::string& teamName) {
    int teamId = nextTeamId++;
    teams[teamId] = {teamId, teamName, {}};
    return teamId;
}

Team& TeamManager::loadTeamFromClub(int clubId) {
    Team team;
    team.teamId = nextTeamId++;
    team.teamName = "Club " + std::to_string(clubId);
    team.players = teamRepo.fetchPlayersForClub(clubId);
    teams[team.teamId] = team;
    return teams[team.teamId];
}

std::vector<Team> TeamManager::getAllTeams() {
    std::vector<Team> teamList;
    for (const auto& pair : teams) {
        teamList.push_back(pair.second);
    }
    return teamList;
}

bool TeamManager::addPlayerToTeam(int teamId, const Player& player) {
    if (teams.find(teamId) == teams.end()) return false;
    teams[teamId].players.push_back(player);
    return true;
}

bool TeamManager::removePlayerFromTeam(int teamId, int playerId) {
    if (teams.find(teamId) == teams.end()) return false;

    auto& players = teams[teamId].players;
    players.erase(
        std::remove_if(
            players.begin(), players.end(), [playerId](const Player& p) { 
                return p.playerId == playerId; 
            }
        ), players.end()
    );
    return true;
}

std::vector<std::string> TeamManager::getMissingPositions(const Team& team) {
    std::vector<std::string> missingPositions;

    std::vector<std::string> availableSubPositions = getAvailableSubPositions();
    
    std::unordered_map<std::string, int> requiredPositions;
    for (const auto& position : availableSubPositions) {
        requiredPositions[position] = 1;  
    }

    for (const auto& player : team.players) {
        if (requiredPositions.find(player.subPosition) != requiredPositions.end()) {
            requiredPositions[player.subPosition]--;
        }
    }

    for (const auto& [position, count] : requiredPositions) {
        if (count > 0) {
            missingPositions.push_back(position);
        }
    }

    return missingPositions;
}


void TeamManager::autoFillTeam(Team& team, int budget) {
    std::vector<std::string> missingPositions = getMissingPositions(team);
    if (missingPositions.empty()) return;

    std::vector<std::pair<int, Player>> selectedPlayers =
        ratingManager.selectOptimalTeamByPositions(missingPositions, budget);

    for (const auto& [positionIndex, player] : selectedPlayers) {
        addPlayerToTeam(team.teamId, player);
    }
}

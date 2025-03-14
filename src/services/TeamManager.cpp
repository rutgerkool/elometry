#include "services/TeamManager.h"
#include <iostream>

TeamManager::TeamManager(TeamRepository& teamRepo, RatingManager& ratingManager, PlayerRepository& playerRepo)
    : teamRepo(teamRepo), ratingManager(ratingManager), playerRepo(playerRepo) {}

std::vector<std::string> TeamManager::getAvailableSubPositions() {
    return teamRepo.getAvailableSubPositions();
}

Team& TeamManager::createTeam(const std::string& teamName) {
    int teamId = ++nextTeamId;
    teams[teamId] = {teamId, teamName, {}};
    return teams[teamId];
}

Team& TeamManager::loadTeamFromClub(int clubId) {
    std::string clubName = "Club " + std::to_string(clubId);
    
    std::vector<std::pair<int, std::string>> clubs = getAllClubs();
    
    for (const auto& [id, name] : clubs) {
        if (id == clubId) {
            clubName = name;
            break;
        }
    }
    
    Team team;
    std::vector<Player> selection = playerRepo.fetchPlayers(clubId);
    team.teamId = ++nextTeamId;
    team.teamName = clubName; 
    team.players = ratingManager.getFilteredRatedPlayers(selection);  
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

    teamRepo.removePlayerFromAllLineups(teamId, playerId);
    
    std::vector<Lineup> lineups = getTeamLineups(teamId);
    for (auto& lineup : lineups) {
        auto& positions = lineup.playerPositions;
        positions.erase(
            std::remove_if(positions.begin(), positions.end(),
                [playerId](const PlayerPosition& pos) { return pos.playerId == playerId; }
            ),
            positions.end()
        );
    }

    auto& players = teams[teamId].players;
    players.erase(
        std::remove_if(players.begin(), players.end(),
            [playerId](const Player& p) { return p.playerId == playerId; }
        ),
        players.end()
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

void TeamManager::autoFillTeam(Team& team, int64_t budget) {
    std::vector<std::string> missingPositions = getMissingPositions(team);
    if (missingPositions.empty()) return;

    std::vector<std::pair<int, Player>> selectedPlayers =
        ratingManager.selectOptimalTeamByPositions(missingPositions, budget);

    for (const auto& [positionIndex, player] : selectedPlayers) {
        addPlayerToTeam(team.teamId, player);
    }
}

Team& TeamManager::loadTeam(int teamId) {
    auto it = teams.find(teamId);
    if (it == teams.end()) {
        throw std::runtime_error("Team not found");
    }
    return it->second;
}

bool TeamManager::deleteTeam(int teamId) {
    teamRepo.deleteTeam(teamId);
    return teams.erase(teamId) > 0;
}

bool TeamManager::updateTeamName(int teamId, const std::string& newName) {
    auto it = teams.find(teamId);
    if (it == teams.end()) {
        return false;
    }
    
    bool success = teamRepo.updateTeamName(teamId, newName);
    
    if (success) {
        it->second.teamName = newName;
    }
    
    return success;
}

void TeamManager::setTeamBudget(int teamId, int64_t newBudget) {
    if (teams.find(teamId) == teams.end()) {
        throw std::runtime_error("Team not found");
    }
    teams[teamId].budget = newBudget;
}

Player TeamManager::searchPlayerById(int playerId) {
    std::vector<Player> players = playerRepo.fetchPlayers(-1, playerId);
    if (players.empty()) {
        throw std::runtime_error("Player not found");
    }
    return players.front();
}

std::vector<std::pair<int, std::string>> TeamManager::getAllClubs() {
    std::vector<std::pair<int, std::string>> clubs;
    std::unordered_map<int, std::string> uniqueClubs;
    
    std::vector<Player> players = playerRepo.fetchPlayers();
    
    for (const auto& player : players) {
        if (player.clubId > 0 && !uniqueClubs.count(player.clubId)) {
            uniqueClubs[player.clubId] = player.clubName;
        }
    }
    
    for (const auto& [id, name] : uniqueClubs) {
        clubs.emplace_back(id, name);
    }
    
    std::sort(clubs.begin(), clubs.end(), 
        [](const auto& a, const auto& b) { return a.second < b.second; });
    
    return clubs;
}

void TeamManager::saveTeam(const Team& team) {
    teamRepo.createTeam(team);
    for (const auto& player : team.players) {
        teamRepo.addPlayerToTeam(team.teamId, player.playerId);
    }
}

void TeamManager::saveTeamPlayers(const Team& team) {
    teamRepo.removeAllPlayersFromTeam(team.teamId);
    for (const auto& player : team.players) {
        teamRepo.addPlayerToTeam(team.teamId, player.playerId);
    }
}

void TeamManager::loadTeams() {
    teams.clear();
    std::vector<Team> loadedTeams = teamRepo.getAllTeams();
    for (const auto& team : loadedTeams) {
        std::vector<Player> selection = playerRepo.fetchPlayers(-1, -1, team.teamId);
        teams[team.teamId] = team;
        teams[team.teamId].players = ratingManager.getFilteredRatedPlayers(selection);
        nextTeamId = std::max(nextTeamId, team.teamId);
    }
}

std::vector<Formation> TeamManager::getAllFormations() {
    return teamRepo.getAllFormations();
}

Lineup TeamManager::createLineup(int teamId, int formationId, const std::string& lineupName) {
    int lineupId = teamRepo.createLineup(teamId, formationId, lineupName);
    Lineup lineup;
    lineup.lineupId = lineupId;
    lineup.teamId = teamId;
    lineup.formationId = formationId;
    lineup.name = lineupName;

    std::vector<Formation> formations = getAllFormations();
    for (const auto& formation : formations) {
        if (formation.id == formationId) {
            lineup.formationName = formation.name;
            break;
        }
    }

    lineup.isActive = true;
    
    auto it = teams.find(teamId);
    if (it != teams.end()) {
        for (const auto& player : it->second.players) {
            PlayerPosition playerPos;
            playerPos.playerId = player.playerId;
            playerPos.positionType = PositionType::RESERVE;
            playerPos.fieldPosition = "";
            playerPos.order = 0;
            lineup.playerPositions.push_back(playerPos);
        }
    }
    
    saveLineup(lineup);
    return lineup;
}

Lineup TeamManager::getActiveLineup(int teamId) {
    return teamRepo.getActiveLineup(teamId);
}

bool TeamManager::setActiveLineup(int teamId, int lineupId) {
    return teamRepo.setActiveLineup(teamId, lineupId);
}

bool TeamManager::updatePlayerPosition(int lineupId, int playerId, PositionType positionType, const std::string& fieldPosition, int order) {
    return teamRepo.updatePlayerPosition(lineupId, playerId, positionType, fieldPosition, order);
}

bool TeamManager::saveLineup(const Lineup& lineup) {
    return teamRepo.saveLineup(lineup);
}

bool TeamManager::deleteLineup(int lineupId) {
    return teamRepo.deleteLineup(lineupId);
}

std::vector<Lineup> TeamManager::getTeamLineups(int teamId) {
    return teamRepo.getTeamLineups(teamId);
}

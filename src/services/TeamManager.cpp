#include "services/TeamManager.h"

#include <algorithm>
#include <ranges>
#include <iterator>
#include <stdexcept>

TeamManager::TeamManager(TeamRepository& teamRepo, RatingManager& ratingManager, PlayerRepository& playerRepo)
    : m_teamRepo(teamRepo)
    , m_ratingManager(ratingManager)
    , m_playerRepo(playerRepo) 
{}

std::vector<std::string> TeamManager::getAvailableSubPositions() const {
    return m_teamRepo.getAvailableSubPositions();
}

Team& TeamManager::createTeam(std::string_view teamName) {
    int teamId = ++m_nextTeamId;
    m_teams[teamId] = Team(teamId, std::string{teamName});
    return m_teams[teamId];
}

Team& TeamManager::loadTeamFromClub(int clubId) {
    auto clubs = getAllClubs();
    
    auto clubIt = std::ranges::find_if(clubs, [clubId](const auto& pair) {
        return pair.first == clubId;
    });
    
    std::string clubName = clubIt != clubs.end() 
        ? clubIt->second 
        : "Club " + std::to_string(clubId);
    
    std::vector<Player> clubPlayers = m_playerRepo.fetchPlayers(clubId);
    std::vector<Player> ratedPlayers = m_ratingManager.getFilteredRatedPlayers(clubPlayers);
    
    int newTeamId = ++m_nextTeamId;
    Team& team = m_teams[newTeamId] = Team(newTeamId, std::move(clubName));
    team.players = std::move(ratedPlayers);
    
    return team;
}

std::vector<Team> TeamManager::getAllTeams() const {
    std::vector<Team> teamList;
    teamList.reserve(m_teams.size());
    
    for (const auto& [_, team] : m_teams) {
        teamList.push_back(team);
    }
    
    return teamList;
}

bool TeamManager::addPlayerToTeam(int teamId, const Player& player) {
    auto teamIt = m_teams.find(teamId);
    if (teamIt == m_teams.end()) {
        return false;
    }
    
    teamIt->second.players.push_back(player);
    return true;
}

bool TeamManager::removePlayerFromTeam(int teamId, int playerId) {
    auto teamIt = m_teams.find(teamId);
    if (teamIt == m_teams.end()) {
        return false;
    }
    
    m_teamRepo.removePlayerFromAllLineups(teamId, playerId);
    
    auto& players = teamIt->second.players;
    const auto playerIt = std::ranges::find_if(players, [playerId](const Player& p) {
        return p.playerId == playerId;
    });
    
    if (playerIt == players.end()) {
        return false;
    }
    
    players.erase(playerIt);
    return true;
}

std::unordered_map<std::string, int> TeamManager::buildRequiredPositionsMap(
    const std::vector<std::string>& availableSubPositions) const {
    
    std::unordered_map<std::string, int> requiredPositions;
    for (const auto& position : availableSubPositions) {
        requiredPositions[position] = 1;
    }
    
    return requiredPositions;
}

std::unordered_map<std::string, bool> TeamManager::mapTeamPositions(
    const Team& team, 
    const std::unordered_map<std::string, int>& requiredPositions) const {
    
    std::unordered_map<std::string, bool> positionMap;
    
    for (const auto& [position, _] : requiredPositions) {
        positionMap[position] = false;
    }
    
    for (const auto& player : team.players) {
        if (requiredPositions.contains(player.subPosition)) {
            positionMap[player.subPosition] = true;
        }
    }
    
    return positionMap;
}

std::vector<std::string> TeamManager::extractMissingPositions(
    const std::unordered_map<std::string, bool>& positionMap) const {
    
    std::vector<std::string> missingPositions;
    
    for (const auto& [position, filled] : positionMap) {
        if (!filled) {
            missingPositions.push_back(position);
        }
    }
    
    return missingPositions;
}

std::vector<std::string> TeamManager::getMissingPositions(const Team& team) const {
    std::vector<std::string> availableSubPositions = getAvailableSubPositions();
    auto requiredPositions = buildRequiredPositionsMap(availableSubPositions);
    auto positionMap = mapTeamPositions(team, requiredPositions);
    
    return extractMissingPositions(positionMap);
}

void TeamManager::autoFillTeam(Team& team, int64_t budget) {
    std::vector<std::string> missingPositions = getMissingPositions(team);
    if (missingPositions.empty()) {
        return;
    }

    auto selectedPlayers = m_ratingManager.selectOptimalTeamByPositions(
        missingPositions, budget);
    
    for (const auto& [_, player] : selectedPlayers) {
        addPlayerToTeam(team.teamId, player);
    }
}

Team& TeamManager::loadTeam(int teamId) {
    auto it = m_teams.find(teamId);
    if (it == m_teams.end()) {
        throw std::runtime_error("Team not found");
    }
    
    return it->second;
}

bool TeamManager::deleteTeam(int teamId) {
    m_teamRepo.deleteTeam(teamId);
    return m_teams.erase(teamId) > 0;
}

bool TeamManager::updateTeamName(int teamId, std::string_view newName) {
    auto it = m_teams.find(teamId);
    if (it == m_teams.end()) {
        return false;
    }
    
    if (m_teamRepo.updateTeamName(teamId, std::string{newName})) {
        it->second.teamName = std::string{newName};
        return true;
    }
    
    return false;
}

void TeamManager::setTeamBudget(int teamId, int64_t newBudget) {
    auto it = m_teams.find(teamId);
    if (it == m_teams.end()) {
        throw std::runtime_error("Team not found");
    }
    
    it->second.budget = newBudget;
}

Player TeamManager::searchPlayerById(int playerId) const {
    std::vector<Player> players = m_playerRepo.fetchPlayers(-1, playerId);
    if (players.empty()) {
        throw std::runtime_error("Player not found");
    }
    
    return players.front();
}

std::vector<std::pair<int, std::string>> TeamManager::getAllClubs() const {
    std::vector<std::pair<int, std::string>> clubs;
    std::unordered_map<int, std::string> uniqueClubs;
    
    std::vector<Player> players = m_playerRepo.fetchPlayers();
    
    for (const auto& player : players) {
        if (player.clubId > 0 && !uniqueClubs.contains(player.clubId)) {
            uniqueClubs[player.clubId] = player.clubName;
        }
    }
    
    clubs.reserve(uniqueClubs.size());
    for (const auto& [id, name] : uniqueClubs) {
        clubs.emplace_back(id, name);
    }
    
    std::ranges::sort(clubs, {}, &std::pair<int, std::string>::second);
    
    return clubs;
}

void TeamManager::saveTeam(const Team& team) {
    m_teamRepo.createTeam(team);
    for (const auto& player : team.players) {
        m_teamRepo.addPlayerToTeam(team.teamId, player.playerId);
    }
}

void TeamManager::saveTeamPlayers(const Team& team) {
    m_teamRepo.removeAllPlayersFromTeam(team.teamId);
    for (const auto& player : team.players) {
        m_teamRepo.addPlayerToTeam(team.teamId, player.playerId);
    }
}

void TeamManager::loadTeams() {
    m_teams.clear();
    std::vector<Team> loadedTeams = m_teamRepo.getAllTeams();
    
    for (auto&& team : loadedTeams) {
        int teamId = team.teamId;
        std::vector<Player> selection = m_playerRepo.fetchPlayers(-1, -1, teamId);
        auto ratedPlayers = m_ratingManager.getFilteredRatedPlayers(selection);
        
        m_teams[teamId] = std::move(team);
        m_teams[teamId].players = std::move(ratedPlayers);
        
        m_nextTeamId = std::max(m_nextTeamId, teamId);
    }
}

std::vector<Formation> TeamManager::getAllFormations() const {
    return m_teamRepo.getAllFormations();
}

Lineup TeamManager::createLineup(int teamId, int formationId, std::string_view lineupName) {
    int lineupId = m_teamRepo.createLineup(teamId, formationId, std::string{lineupName});
    
    Lineup lineup;
    lineup.lineupId = lineupId;
    lineup.teamId = teamId;
    lineup.formationId = formationId;
    lineup.name = std::string{lineupName};
    lineup.isActive = true;
    
    auto formationsVec = getAllFormations();
    auto formationIt = std::ranges::find_if(formationsVec, [formationId](const Formation& f) {
        return f.id == formationId;
    });
    
    if (formationIt != formationsVec.end()) {
        lineup.formationName = formationIt->name;
    }
    
    auto teamIt = m_teams.find(teamId);
    if (teamIt != m_teams.end()) {
        for (const auto& player : teamIt->second.players) {
            PlayerPosition pos;
            pos.playerId = player.playerId;
            pos.positionType = PositionType::RESERVE;
            pos.fieldPosition = "";
            pos.order = 0;
            
            lineup.playerPositions.push_back(pos);
        }
    }
    
    saveLineup(lineup);
    return lineup;
}

Lineup TeamManager::getActiveLineup(int teamId) const {
    return m_teamRepo.getActiveLineup(teamId);
}

bool TeamManager::setActiveLineup(int teamId, int lineupId) {
    return m_teamRepo.setActiveLineup(teamId, lineupId);
}

bool TeamManager::updatePlayerPosition(int lineupId, int playerId, PositionType positionType, 
                                     std::string_view fieldPosition, int order) {
    return m_teamRepo.updatePlayerPosition(
        lineupId, playerId, positionType, std::string{fieldPosition}, order);
}

bool TeamManager::saveLineup(const Lineup& lineup) {
    return m_teamRepo.saveLineup(lineup);
}

bool TeamManager::deleteLineup(int lineupId) {
    return m_teamRepo.deleteLineup(lineupId);
}

std::vector<Lineup> TeamManager::getTeamLineups(int teamId) const {
    return m_teamRepo.getTeamLineups(teamId);
}

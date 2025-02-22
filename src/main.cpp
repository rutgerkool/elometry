#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/repositories/TeamRepository.h"
#include <iostream>

int main() {
    Database database("test.db");
    RatingManager ratingManager(database);
    TeamRepository teamRepository(database);
    TeamManager teamManager(teamRepository, ratingManager);

    ratingManager.loadAndProcessRatings();

    Team& team = teamManager.createTeam("My Team");
    teamManager.addPlayerToTeam(team.teamId, {1, 1, "Player 1", "My Club", "Centre-Back", "Defender", "2026-06-30", 7000000, 10000000});
    teamManager.addPlayerToTeam(team.teamId, {2, 1, "Player 2", "My Club", "Centre-Forward", "Attack", "2027-01-30", 4000000, 8000000});
    
    std::cout << "\nLoaded team with ID: " << team.teamId << " (" << team.teamName << ")\n";

    for (const auto& player : team.players) {
        std::cout << player.name << " (" << player.subPosition << ") - €" 
                  << player.highestMarketValue << "\n";
    }

    std::vector<std::string> missingPositions = teamManager.getMissingPositions(team);
    std::cout << "\nMissing Positions:\n";
    for (const auto& pos : missingPositions) {
        std::cout << " - " << pos << "\n";
    }

    std::cout << "\nAuto-filling missing positions...\n";
    teamManager.autoFillTeam(team, 20000000);

    std::cout << "\nFinal Team Roster:\n";
    for (const auto& player : team.players) {
        std::cout << player.name << " (" << player.subPosition << ") - €" 
                  << player.highestMarketValue << "\n";
    }

    return 0;
}

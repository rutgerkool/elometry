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

    int clubId = 610;
    Team& team = teamManager.loadTeamFromClub(clubId);
    std::cout << "\nLoaded team for Club ID: " << clubId << " (" << team.teamName << ")\n";

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
    teamManager.autoFillTeam(team, 50000000);

    std::cout << "\nFinal Team Roster:\n";
    for (const auto& player : team.players) {
        std::cout << player.name << " (" << player.subPosition << ") - €" 
                  << player.highestMarketValue << "\n";
    }

    return 0;
}

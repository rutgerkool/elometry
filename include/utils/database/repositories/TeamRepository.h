#ifndef TEAMREPOSITORY_H
#define TEAMREPOSITORY_H

#include "utils/database/Database.h"
#include "models/PlayerRating.h"
#include <vector>

struct Team {
    int teamId;
    std::string teamName;
    std::vector<Player> players;
    int64_t budget = 20000000;
};

class TeamRepository {
public:
    explicit TeamRepository(Database& database);

    std::vector<std::string> getAvailableSubPositions();

    void createTeam(const Team& team);
    std::vector<Team> getAllTeams();
    void addPlayerToTeam(int teamId, int playerId);
    void removePlayerFromTeam(int teamId, int playerId);
    void removeAllPlayersFromTeam(int teamId);
    void deleteTeam(int teamId);
    bool updateTeamName(int teamId, const std::string& newName);

private:
    sqlite3* db;
};

#endif

#ifndef TEAMREPOSITORY_H
#define TEAMREPOSITORY_H

#include "utils/database/Database.h"
#include "models/PlayerRating.h"
#include "gui/models/LineupTypes.h"
#include <vector>
#include <string>

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

        std::vector<Formation> getAllFormations();
        int createLineup(int teamId, int formationId, const std::string& lineupName = "");
        Lineup getActiveLineup(int teamId);
        bool setActiveLineup(int teamId, int lineupId);
        bool updatePlayerPosition(int lineupId, int playerId, PositionType positionType, const std::string& fieldPosition = "", int order = 0);
        bool saveLineup(const Lineup& lineup);
        bool deleteLineup(int lineupId);
        std::vector<Lineup> getTeamLineups(int teamId);
        void removePlayerFromAllLineups(int teamId, int playerId);

    private:
        sqlite3* db;
        std::string positionTypeToString(PositionType positionType);
        PositionType stringToPositionType(const std::string& positionType);
        
        bool executeStatement(const std::string& query, const std::vector<std::pair<int, int>>& intBindings = {}, 
                             const std::vector<std::pair<int, std::string>>& textBindings = {});
        bool executeStatement(sqlite3_stmt* stmt, const std::vector<std::pair<int, int>>& intBindings = {}, 
                             const std::vector<std::pair<int, std::string>>& textBindings = {});
        bool prepareStatement(const std::string& query, sqlite3_stmt** stmt);
        bool deactivateTeamLineups(int teamId);
        bool fillLineupPlayerPositions(Lineup& lineup);
        bool saveLineupPlayers(const Lineup& lineup);
};

#endif

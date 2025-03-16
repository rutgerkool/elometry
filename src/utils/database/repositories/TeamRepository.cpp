#include "utils/database/repositories/TeamRepository.h"
#include "utils/database/PlayerMapper.h"
#include "utils/Season.h"
#include <iostream>

TeamRepository::TeamRepository(Database& database) {
    db = database.getConnection();
}

bool TeamRepository::prepareStatement(const std::string& query, sqlite3_stmt** stmt) {
    return sqlite3_prepare_v2(db, query.c_str(), -1, stmt, nullptr) == SQLITE_OK;
}

bool TeamRepository::executeStatement(const std::string& query, 
                                     const std::vector<std::pair<int, int>>& intBindings,
                                     const std::vector<std::pair<int, std::string>>& textBindings) {
    sqlite3_stmt* stmt;
    if (!prepareStatement(query, &stmt)) {
        return false;
    }

    bool result = executeStatement(stmt, intBindings, textBindings);
    sqlite3_finalize(stmt);
    return result;
}

bool TeamRepository::executeStatement(sqlite3_stmt* stmt, 
                                     const std::vector<std::pair<int, int>>& intBindings,
                                     const std::vector<std::pair<int, std::string>>& textBindings) {
    for (const auto& [index, value] : intBindings) {
        sqlite3_bind_int(stmt, index, value);
    }
    
    for (const auto& [index, value] : textBindings) {
        if (value.empty()) {
            sqlite3_bind_null(stmt, index);
        } else {
            sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
        }
    }
    
    int result = sqlite3_step(stmt);
    return result == SQLITE_DONE;
}

std::vector<std::string> TeamRepository::getAvailableSubPositions() {
    std::vector<std::string> subPositions;
    sqlite3_stmt* stmt;
    std::string query = "SELECT DISTINCT sub_position FROM players WHERE sub_position IS NOT NULL;";

    if (prepareStatement(query, &stmt)) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            subPositions.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
    }

    sqlite3_finalize(stmt);
    return subPositions;
}

void TeamRepository::createTeam(const Team& team) {
    std::string query = "INSERT INTO teams (team_id, team_name) VALUES (?, ?);";
    executeStatement(query, 
                    {{1, team.teamId}}, 
                    {{2, team.teamName}});
}

std::vector<Team> TeamRepository::getAllTeams() {
    std::vector<Team> teams;
    std::string query = "SELECT team_id, team_name FROM teams;";
    sqlite3_stmt* stmt;

    if (prepareStatement(query, &stmt)) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int teamId = sqlite3_column_int(stmt, 0);
            std::string teamName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            teams.push_back({teamId, teamName, {}});
        }
    }
    sqlite3_finalize(stmt);
    return teams;
}

void TeamRepository::addPlayerToTeam(int teamId, int playerId) {
    std::string query = "INSERT INTO team_players (team_id, player_id) VALUES (?, ?);";
    executeStatement(query, {{1, teamId}, {2, playerId}}, {});
}

void TeamRepository::removePlayerFromTeam(int teamId, int playerId) {
    std::string query = "DELETE FROM team_players WHERE team_id = ? AND player_id = ?;";
    executeStatement(query, {{1, teamId}, {2, playerId}}, {});
}

void TeamRepository::deleteTeam(int teamId) {
    std::string query = "DELETE FROM teams WHERE team_id = ?;";
    executeStatement(query, {{1, teamId}}, {});
}

void TeamRepository::removeAllPlayersFromTeam(int teamId) {
    std::string query = "DELETE FROM team_players WHERE team_id = ?;";
    executeStatement(query, {{1, teamId}}, {});
}

bool TeamRepository::updateTeamName(int teamId, const std::string& newName) {
    std::string query = "UPDATE teams SET team_name = ? WHERE team_id = ?;";
    sqlite3_stmt* stmt;
    bool success = false;

    if (prepareStatement(query, &stmt)) {
        sqlite3_bind_text(stmt, 1, newName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, teamId);
        
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            success = sqlite3_changes(db) > 0;
        }
    }
    sqlite3_finalize(stmt);
    return success;
}

std::string TeamRepository::positionTypeToString(PositionType positionType) {
    switch (positionType) {
        case PositionType::STARTING: return "STARTING";
        case PositionType::BENCH: return "BENCH";
        case PositionType::RESERVE: return "RESERVE";
        default: return "RESERVE";
    }
}

PositionType TeamRepository::stringToPositionType(const std::string& positionType) {
    if (positionType == "STARTING") return PositionType::STARTING;
    if (positionType == "BENCH") return PositionType::BENCH;
    return PositionType::RESERVE;
}

std::vector<Formation> TeamRepository::getAllFormations() {
    std::vector<Formation> formations;
    sqlite3_stmt* stmt;
    std::string query = "SELECT formation_id, formation_name, description FROM team_formations;";

    if (prepareStatement(query, &stmt)) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Formation formation;
            formation.id = sqlite3_column_int(stmt, 0);
            formation.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            
            if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
                formation.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            }
            
            formations.push_back(formation);
        }
    }
    
    sqlite3_finalize(stmt);
    return formations;
}

bool TeamRepository::deactivateTeamLineups(int teamId) {
    std::string query = "UPDATE team_lineups SET is_active = 0 WHERE team_id = ?;";
    return executeStatement(query, {{1, teamId}}, {});
}

int TeamRepository::createLineup(int teamId, int formationId, const std::string& lineupName) {
    deactivateTeamLineups(teamId);
    
    std::string query = "INSERT INTO team_lineups (team_id, formation_id, is_active, lineup_name) VALUES (?, ?, 1, ?);";
    sqlite3_stmt* stmt;
    int lineupId = -1;
    
    if (prepareStatement(query, &stmt)) {
        sqlite3_bind_int(stmt, 1, teamId);
        sqlite3_bind_int(stmt, 2, formationId);
        sqlite3_bind_text(stmt, 3, lineupName.c_str(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            lineupId = static_cast<int>(sqlite3_last_insert_rowid(db));
        }
    }
    sqlite3_finalize(stmt);
    
    return lineupId;
}

Lineup TeamRepository::getActiveLineup(int teamId) {
    Lineup lineup;
    lineup.lineupId = -1;
    
    std::string query = R"(
        SELECT 
            l.lineup_id,
            l.formation_id,
            f.formation_name,
            l.lineup_name
        FROM team_lineups l
        JOIN team_formations f
            ON l.formation_id = f.formation_id
        WHERE l.team_id = ? AND l.is_active = 1;
    )";
    sqlite3_stmt* stmt;
    
    if (prepareStatement(query, &stmt)) {
        sqlite3_bind_int(stmt, 1, teamId);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            lineup.lineupId = sqlite3_column_int(stmt, 0);
            lineup.teamId = teamId;
            lineup.formationId = sqlite3_column_int(stmt, 1);
            lineup.formationName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            
            if (sqlite3_column_type(stmt, 3) != SQLITE_NULL) {
                lineup.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            }
            
            lineup.isActive = true;
        }
    }
    sqlite3_finalize(stmt);
    
    if (lineup.lineupId > 0) {
        fillLineupPlayerPositions(lineup);
    }
    
    return lineup;
}

bool TeamRepository::fillLineupPlayerPositions(Lineup& lineup) {
    std::string query = R"(
        SELECT 
            player_id, 
            position_type, 
            field_position, 
            position_order
        FROM lineup_players
        WHERE lineup_id = ?;
    )";
    sqlite3_stmt* stmt;
    
    if (!prepareStatement(query, &stmt)) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, lineup.lineupId);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PlayerPosition playerPos;
        playerPos.playerId = sqlite3_column_int(stmt, 0);
        
        std::string posType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        playerPos.positionType = stringToPositionType(posType);
        
        if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
            playerPos.fieldPosition = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        }
        
        playerPos.order = sqlite3_column_int(stmt, 3);
        
        lineup.playerPositions.push_back(playerPos);
    }
    
    sqlite3_finalize(stmt);
    return true;
}

bool TeamRepository::setActiveLineup(int teamId, int lineupId) {
    if (!deactivateTeamLineups(teamId)) {
        return false;
    }
    
    std::string query = "UPDATE team_lineups SET is_active = 1 WHERE lineup_id = ? AND team_id = ?;";
    return executeStatement(query, {{1, lineupId}, {2, teamId}}, {});
}

bool TeamRepository::updatePlayerPosition(int lineupId, int playerId, PositionType positionType, 
                                         const std::string& fieldPosition, int order) {
    std::string query = R"(
        INSERT INTO lineup_players (lineup_id, player_id, position_type, field_position, position_order)
        VALUES (?, ?, ?, ?, ?)
        ON CONFLICT(lineup_id, player_id) 
        DO UPDATE SET
            position_type = excluded.position_type,
            field_position = excluded.field_position,
            position_order = excluded.position_order;
    )";
                       
    sqlite3_stmt* stmt;
    
    if (!prepareStatement(query, &stmt)) {
        return false;
    }
    
    std::string posTypeStr = positionTypeToString(positionType);
    
    sqlite3_bind_int(stmt, 1, lineupId);
    sqlite3_bind_int(stmt, 2, playerId);
    sqlite3_bind_text(stmt, 3, posTypeStr.c_str(), -1, SQLITE_TRANSIENT);
    
    if (fieldPosition.empty()) {
        sqlite3_bind_null(stmt, 4);
    } else {
        sqlite3_bind_text(stmt, 4, fieldPosition.c_str(), -1, SQLITE_TRANSIENT);
    }
    
    sqlite3_bind_int(stmt, 5, order);
    
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    
    return success;
}

bool TeamRepository::saveLineup(const Lineup& lineup) {
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    
    bool success = true;
    
    std::string query = R"(
        INSERT INTO team_lineups (lineup_id, team_id, formation_id, is_active, lineup_name)
        VALUES (?, ?, ?, ?, ?)
        ON CONFLICT(lineup_id) 
            DO UPDATE SET
            formation_id = excluded.formation_id,
            is_active = excluded.is_active,
            lineup_name = excluded.lineup_name;
    )";
    
    sqlite3_stmt* stmt;
    
    if (prepareStatement(query, &stmt)) {
        sqlite3_bind_int(stmt, 1, lineup.lineupId);
        sqlite3_bind_int(stmt, 2, lineup.teamId);
        sqlite3_bind_int(stmt, 3, lineup.formationId);
        sqlite3_bind_int(stmt, 4, lineup.isActive ? 1 : 0);
        
        if (lineup.name.empty()) {
            sqlite3_bind_null(stmt, 5);
        } else {
            sqlite3_bind_text(stmt, 5, lineup.name.c_str(), -1, SQLITE_TRANSIENT);
        }
        
        success = (sqlite3_step(stmt) == SQLITE_DONE);
    } else {
        success = false;
    }
    
    sqlite3_finalize(stmt);
    
    if (success && lineup.isActive) {
        std::string query = "UPDATE team_lineups SET is_active = 0 WHERE team_id = ? AND lineup_id != ?;";
        success = executeStatement(query, {{1, lineup.teamId}, {2, lineup.lineupId}}, {});
    }
    
    if (success) {
        std::string query = "DELETE FROM lineup_players WHERE lineup_id = ?;";
        success = executeStatement(query, {{1, lineup.lineupId}}, {});
    }
    
    if (success) {
        success = saveLineupPlayers(lineup);
    }
    
    if (success) {
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    } else {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
    }
    
    return success;
}

bool TeamRepository::saveLineupPlayers(const Lineup& lineup) {
    std::string query = R"(
        INSERT INTO lineup_players (lineup_id, player_id, position_type, field_position, position_order)
        VALUES (?, ?, ?, ?, ?);
    )";
    
    sqlite3_stmt* stmt;
    bool success = prepareStatement(query, &stmt);
    
    if (!success) {
        return false;
    }
    
    for (const auto& playerPos : lineup.playerPositions) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        
        sqlite3_bind_int(stmt, 1, lineup.lineupId);
        sqlite3_bind_int(stmt, 2, playerPos.playerId);
        sqlite3_bind_text(stmt, 3, positionTypeToString(playerPos.positionType).c_str(), -1, SQLITE_TRANSIENT);
        
        if (playerPos.fieldPosition.empty()) {
            sqlite3_bind_null(stmt, 4);
        } else {
            sqlite3_bind_text(stmt, 4, playerPos.fieldPosition.c_str(), -1, SQLITE_TRANSIENT);
        }
        
        sqlite3_bind_int(stmt, 5, playerPos.order);
        
        int result = sqlite3_step(stmt);
        if (result != SQLITE_DONE) {
            std::cerr << "SQL error in saveLineup: " << sqlite3_errmsg(db) 
                      << " for player: " << playerPos.playerId 
                      << " position: " << positionTypeToString(playerPos.positionType) 
                      << std::endl;
            success = false;
            break;
        }
    }
    
    sqlite3_finalize(stmt);
    return success;
}

bool TeamRepository::deleteLineup(int lineupId) {
    std::string query = "DELETE FROM team_lineups WHERE lineup_id = ?;";
    return executeStatement(query, {{1, lineupId}}, {});
}

std::vector<Lineup> TeamRepository::getTeamLineups(int teamId) {
    std::vector<Lineup> lineups;
    
    std::string query = R"(
        SELECT l.lineup_id, l.formation_id, f.formation_name, l.is_active, l.lineup_name
        FROM team_lineups l
        JOIN team_formations f 
            ON l.formation_id = f.formation_id
        WHERE l.team_id = ?;
    )";
                       
    sqlite3_stmt* stmt;
    
    if (prepareStatement(query, &stmt)) {
        sqlite3_bind_int(stmt, 1, teamId);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Lineup lineup;
            lineup.lineupId = sqlite3_column_int(stmt, 0);
            lineup.teamId = teamId;
            lineup.formationId = sqlite3_column_int(stmt, 1);
            lineup.formationName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            lineup.isActive = sqlite3_column_int(stmt, 3) != 0;
            
            if (sqlite3_column_type(stmt, 4) != SQLITE_NULL) {
                lineup.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            }
            
            lineups.push_back(lineup);
        }
    }
    
    sqlite3_finalize(stmt);
    
    for (auto& lineup : lineups) {
        fillLineupPlayerPositions(lineup);
    }
    
    return lineups;
}

void TeamRepository::removePlayerFromAllLineups(int teamId, int playerId) {
    std::string query = R"(
        DELETE FROM lineup_players 
        WHERE player_id = ? AND lineup_id IN (
            SELECT lineup_id 
            FROM team_lineups
            WHERE team_id = ?
        );
    )";
    
    executeStatement(query, {{1, playerId}, {2, teamId}}, {});
}

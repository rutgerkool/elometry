#include "utils/database/repositories/TeamRepository.h"
#include "utils/database/PlayerMapper.h"
#include "utils/Season.h"
#include <iostream>

TeamRepository::TeamRepository(Database& database) {
    db = database.getConnection();
}

std::vector<std::string> TeamRepository::getAvailableSubPositions() {
    std::vector<std::string> subPositions;
    sqlite3_stmt *stmt;
    std::string query = "SELECT DISTINCT sub_position FROM players WHERE sub_position IS NOT NULL;";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            subPositions.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
    }

    sqlite3_finalize(stmt);
    return subPositions;
}

std::vector<Player> TeamRepository::fetchPlayersForClub(int clubId) {
    std::vector<Player> players;
    sqlite3_stmt *stmt;

    int currentSeasonYear = getCurrentSeasonYear();

    std::string query = R"(
        SELECT 
            player_id, 
            current_club_id, 
            name, 
            current_club_name, 
            sub_position, 
            position, 
            contract_expiration_date, 
            market_value_in_eur, 
            highest_market_value_in_eur,
            image_url
        FROM players 
        WHERE current_club_id = ? AND last_season = ?;
    )";

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, clubId);
        sqlite3_bind_int(stmt, 2, currentSeasonYear);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Player player = PlayerMapper::mapPlayerFromStatement(stmt);
            players.push_back(player);
        }
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return players;
}

void TeamRepository::createTeam(const std::string& teamName) {
    std::string query = "INSERT INTO teams (team_name) VALUES (?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, teamName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

std::vector<Team> TeamRepository::getAllTeams() {
    std::vector<Team> teams;
    std::string query = "SELECT team_id, team_name FROM teams;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
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
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, teamId);
        sqlite3_bind_int(stmt, 2, playerId);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

void TeamRepository::removePlayerFromTeam(int teamId, int playerId) {
    std::string query = "DELETE FROM team_players WHERE team_id = ? AND player_id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, teamId);
        sqlite3_bind_int(stmt, 2, playerId);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

void TeamRepository::deleteTeam(int teamId) {
    std::string query = "DELETE FROM teams WHERE team_id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, teamId);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

void TeamRepository::removeAllPlayersFromTeam(int teamId) {
    std::string query = "DELETE FROM team_players WHERE team_id = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, teamId);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

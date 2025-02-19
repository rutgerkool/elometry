#include "utils/database/repositories/AppearanceRepository.h"

AppearanceRepository::AppearanceRepository(Database& database) {
    db = database.getConnection();
}

std::vector<PlayerAppearance> AppearanceRepository::fetchAppearances() {
    std::vector<PlayerAppearance> appearances;
    sqlite3_stmt * stmt;
    const char * query = "SELECT game_id, player_id, player_club_id, goals, assists, minutes_played FROM appearances";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            PlayerAppearance appearance;
            appearance.gameId = sqlite3_column_int(stmt, 0);
            appearance.playerId = sqlite3_column_int(stmt, 1);
            appearance.clubId = sqlite3_column_int(stmt, 2);
            appearance.goals = sqlite3_column_int(stmt, 3);
            appearance.assists = sqlite3_column_int(stmt, 4);
            appearance.minutesPlayed = sqlite3_column_int(stmt, 5);
            appearances.push_back(appearance);
        }
    }

    sqlite3_finalize(stmt);

    return appearances;
}

#include "utils/database/repositories/GameRepository.h"

GameRepository::GameRepository(Database& database) {
    db = database.getConnection();
}

std::vector<Game> GameRepository::fetchGames() {
    std::vector<Game> games;
    sqlite3_stmt * stmt;
    const char * query = "SELECT game_id, club_id, own_goals, opponent_id, opponent_goals FROM games;";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Game game;
            game.gameId = sqlite3_column_int(stmt, 0);
            game.homeClubId = sqlite3_column_int(stmt, 1);
            game.homeGoals = sqlite3_column_int(stmt, 2);
            game.awayClubId = sqlite3_column_int(stmt, 3);
            game.awayGoals = sqlite3_column_int(stmt, 4);
            games.push_back(game);
        }
    }

    sqlite3_finalize(stmt);

    return games;
}

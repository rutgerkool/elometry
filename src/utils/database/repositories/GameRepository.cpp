#include "utils/database/repositories/GameRepository.h"

GameRepository::GameRepository(Database& database) {
    db = database.getConnection();
}

std::vector<Game> GameRepository::fetchGames() {
    std::vector<Game> games;
    sqlite3_stmt * stmt;
    
    const char * query = R"(
        SELECT 
            cg.game_id, 
            cg.club_id AS home_club_id, 
            cg.own_goals AS home_goals, 
            cg.opponent_id AS away_club_id, 
            cg.opponent_goals AS away_goals,
            h.name AS home_club_name,
            a.name AS away_club_name,
            g.date AS game_date
        FROM 
            club_games cg
        LEFT JOIN 
            clubs h ON cg.club_id = h.club_id
        LEFT JOIN 
            clubs a ON cg.opponent_id = a.club_id
        LEFT JOIN
            games g ON cg.game_id = g.game_id
    )";

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Game game;
            game.gameId = sqlite3_column_int(stmt, 0);
            game.homeClubId = sqlite3_column_int(stmt, 1);
            game.homeGoals = sqlite3_column_int(stmt, 2);
            game.awayClubId = sqlite3_column_int(stmt, 3);
            game.awayGoals = sqlite3_column_int(stmt, 4);
            
            const unsigned char* homeClubName = sqlite3_column_text(stmt, 5);
            const unsigned char* awayClubName = sqlite3_column_text(stmt, 6);
            const unsigned char* dateText = sqlite3_column_text(stmt, 7);
            
            game.homeClubName = homeClubName ? reinterpret_cast<const char*>(homeClubName) : "Unknown";
            game.awayClubName = awayClubName ? reinterpret_cast<const char*>(awayClubName) : "Unknown";
            game.date = dateText ? reinterpret_cast<const char*>(dateText) : "";
            
            games.push_back(game);
        }
    }

    sqlite3_finalize(stmt);

    return games;
}

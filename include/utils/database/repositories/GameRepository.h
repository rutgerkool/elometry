#ifndef GAMEREPOSITORY_H
#define GAMEREPOSITORY_H

#include "utils/database/Database.h"
#include <vector>
#include <string>
#include <optional>

struct Game {
    int gameId{0};
    int homeClubId{0};
    int awayClubId{0};
    int homeGoals{0};
    int awayGoals{0};
    std::string homeClubName;
    std::string awayClubName;
    std::string date;
};

class GameRepository {
public:
    explicit GameRepository(Database& db);
    ~GameRepository() = default;
    
    GameRepository(const GameRepository&) = delete;
    GameRepository& operator=(const GameRepository&) = delete;
    GameRepository(GameRepository&&) = default;
    GameRepository& operator=(GameRepository&&) = default;
    
    [[nodiscard]] std::vector<Game> fetchGames() const;
    [[nodiscard]] std::optional<Game> fetchGameById(int gameId) const;
    [[nodiscard]] std::vector<Game> fetchGamesForClub(int clubId) const;
    [[nodiscard]] std::vector<Game> fetchRecentGames(int limit = 10) const;

private:
    static constexpr auto BASE_QUERY = R"(
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
    
    template<typename... Args>
    [[nodiscard]] std::vector<Game> executeQuery(const std::string& query, Args... args) const;
    
    [[nodiscard]] Game extractGameFromStatement(sqlite3_stmt* stmt) const;
    
    sqlite3* m_db;
};

#endif

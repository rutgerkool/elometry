#ifndef PLAYERRATING_H
#define PLAYERRATING_H

#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"
#include "utils/database/repositories/PlayerRepository.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <deque>
#include <span>

struct RatingChange {
    int gameId;
    double previousRating;
    double newRating;
    std::string opponent;
    bool isHomeGame;
    int minutesPlayed;
    int goalDifference;
    double matchImpact;
    int goals;
    int assists;
    std::string date;
};

struct GameCalculations {
    int gameId{0};
    double homeTeamRating{0.0};
    double awayTeamRating{0.0};
    double homeExpected{0.0};
    double awayExpected{0.0};
    double homeActual{0.0};
    const Game* game{nullptr};
    std::vector<PlayerAppearance> playerAppearances;
};

class PlayerRating {
public:
    explicit PlayerRating(double k = 20.0, double homeAdvantage = 100.0);
    
    void initializePlayer(const Player& player);
    void processMatches(std::span<const Game> games, std::span<const PlayerAppearance> appearances);
    void processMatchesParallel(const std::vector<Game>& games, const std::vector<PlayerAppearance>& appearances);
    
    [[nodiscard]] std::vector<RatingChange> getPlayerRatingHistory(int playerId, int maxGames = 10) const;
    [[nodiscard]] std::vector<std::pair<int, Player>> getSortedRatedPlayers() const;
    
    static bool sortPlayersByRating(const std::pair<int, Player>& a, const std::pair<int, Player>& b);
    
private:
    using PlayerId = int;
    
    static constexpr int MAX_HISTORY_SIZE = 10;
    
    double kFactor;
    double homeAdvantage;
    
    std::unordered_map<PlayerId, Player> ratedPlayers;
    std::unordered_map<PlayerId, std::deque<RatingChange>> ratingHistory;
    
    [[nodiscard]] double calculateExpectation(double playerRating, double opponentTeamRating) const;
    [[nodiscard]] double updateRating(double currentRating, double expected, double actual, int minutesPlayed, int goalDifference) const;
    [[nodiscard]] double calculateMatchImpact(int minutesPlayed, int goalDifference, double actual, double expected) const;
    [[nodiscard]] static double calculateActualResult(int homeGoals, int awayGoals);
    
    void processMatch(const Game& game, std::span<const PlayerAppearance> appearances);
    void calculateTeamRatings(const Game& game, std::span<const PlayerAppearance> appearances, double& homeTeamRating, double& awayTeamRating) const;
    void calculateMatchExpectations(double homeTeamRating, double awayTeamRating, double& homeExpected, double& awayExpected) const;
    void updatePlayerRating(const PlayerAppearance& player, const Game& game, double expected, double actual);
    void createRatingChangeRecord(const PlayerAppearance& player, const Game& game, double previousRating, double newRating, double expected, double actual);
    
    std::unordered_map<int, std::vector<PlayerAppearance>> groupAppearancesByGame(std::span<const PlayerAppearance> appearances) const;
    std::vector<Game> sortGamesByDate(std::span<const Game> games) const;
    std::vector<GameCalculations> prepareGameCalculations(std::span<const Game> sortedGames, const std::unordered_map<int, std::vector<PlayerAppearance>>& gameAppearances) const;
    void applyRatingChanges(std::span<const GameCalculations> calculations);
};

#endif

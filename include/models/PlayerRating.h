#ifndef PLAYERRATING_H
#define PLAYERRATING_H

#include "utils/database/repositories/GameRepository.h"
#include "utils/database/repositories/AppearanceRepository.h"
#include "utils/database/repositories/PlayerRepository.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <deque>

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

class PlayerRating {
    public:
        PlayerRating(double k = 20.0, double homeAdvantage = 100.0);

        void initializePlayer(Player player);
        double calculateExpectation(double playerRating, double opponentTeamRating);
        double updateRating(double currentRating, double expected, double actual, int minutesPlayed, int goalDifference);
        void processMatch(const Game& game, const std::vector<PlayerAppearance>& appearances);
        static bool sortPlayersByRating(const std::pair<int, Player>& a, const std::pair<int, Player>& b);
        void processMatchesParallel(const std::vector<Game>& games, const std::vector<PlayerAppearance>& appearances);
        void saveRatingsToFile();
        std::vector<std::pair<int, Player>> getSortedRatedPlayers();
        std::vector<RatingChange> getPlayerRatingHistory(int playerId, int maxGames = 10);

    private:
        void calculateTeamRatings(const Game& game, const std::vector<PlayerAppearance>& appearances, double& homeTeamRating, double& awayTeamRating);
        void calculateMatchExpectations(double homeTeamRating, double awayTeamRating, double& homeExpected, double& awayExpected);
        double calculateActualResult(int homeGoals, int awayGoals);
        void updatePlayerRating(const PlayerAppearance& player, const Game& game,double expected, double actual);
        void createRatingChangeRecord(const PlayerAppearance& player, const Game& game, double previousRating, double newRating, double expected, double actual);
        double calculateMatchImpact(double kFactor, int minutesPlayed, int goalDifference, double actual, double expected);
        void groupAppearancesByGame(const std::vector<PlayerAppearance>& appearances, std::unordered_map<int, std::vector<PlayerAppearance>>& gameAppearances);
        void sortGamesByDate(const std::vector<Game>& games, std::vector<Game>& sortedGames);
        std::unordered_map<PlayerId, Player> ratedPlayers;
        std::unordered_map<PlayerId, std::deque<RatingChange>> ratingHistory;
        
        double kFactor;
        double homeAdvantage;
        
        static constexpr int MAX_HISTORY_SIZE = 10;
};

#endif

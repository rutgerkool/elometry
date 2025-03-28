#ifndef RATINGMANAGER_H
#define RATINGMANAGER_H

#include "models/PlayerRating.h"
#include "models/ILPSelector.h"

class RatingManager {
    private:
        Database& database;
        PlayerRating ratingSystem;

    public:
        RatingManager(Database& db);

        void loadAndProcessRatings();
        std::vector<std::pair<int, Player>> selectOptimalTeamByPositions(
            const std::vector<std::string>& requiredPositions,
            int64_t budget
        );
        std::vector<Player> getFilteredRatedPlayers(const std::vector<Player>& filterPlayers);
        std::vector<std::pair<int, double>> getRecentRatingProgression(int playerId, int maxGames = 10) const;
        std::vector<Player> getAllPlayers();
        std::vector<RatingChange> getPlayerRatingHistory(int playerId, int maxGames = 10) const {
            return ratingSystem.getPlayerRatingHistory(playerId, maxGames);
        }
        std::vector<std::pair<int, Player>> getSortedRatedPlayers() const {
            return ratingSystem.getSortedRatedPlayers();
        }
};

#endif

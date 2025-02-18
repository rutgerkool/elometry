#ifndef DATALOADER_H
#define DATALOADER_H

#include "models/PlayerRating.h"
#include <vector>

class DataLoader {
    public:
        void loadNumericCellIntoField(std::stringstream& ss, std::string& token, int& field);
        void skipColumns(std::stringstream& ss, std::string& token, int columns = 1);
        std::vector<Game> loadGames(const std::string& filePath);
        std::vector<PlayerAppearance> loadAppearances(const std::string& filePath);
};

#endif

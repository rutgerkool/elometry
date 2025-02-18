#include "utils/DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <locale>

static inline void trim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

void DataLoader::loadNumericCellIntoField(std::stringstream& ss, std::string& token, int& field) {
    getline(ss, token, ',');
    trim(token);
    if (!token.empty()) 
        field = std::stoi(token);
}

void DataLoader::skipColumns(std::stringstream& ss, std::string& token, int columns) {
    while (columns--) {
        getline(ss, token, ',');
    }
}

std::vector<Game> DataLoader::loadGames(const std::string& filePath) {
    std::vector<Game> games;
    std::ifstream file(filePath);
    std::string line;
    getline(file, line);

    while (getline(file, line)) {
        std::stringstream ss(line);
        Game game;
        std::string token;

        try {
            loadNumericCellIntoField(ss, token, game.gameId);
            loadNumericCellIntoField(ss, token, game.homeClubId);
            loadNumericCellIntoField(ss, token, game.homeGoals);

            skipColumns(ss, token, 2);

            loadNumericCellIntoField(ss, token, game.awayClubId);
            loadNumericCellIntoField(ss, token, game.awayGoals);

            skipColumns(ss, token, 2);

            getline(ss, token, ',');
            game.isHome = (token == "Home");

            skipColumns(ss, token);
        } catch (const std::invalid_argument (&e)) {
            std::cerr << "Invalid integer in data: " << token << std::endl;
            continue;
        }

        games.push_back(game);
    }

    return games;
}

std::vector<PlayerAppearance> DataLoader::loadAppearances(const std::string& filePath) {
    std::vector<PlayerAppearance> appearances;
    std::ifstream file(filePath);
    std::string line;
    getline(file, line);

    while (getline(file, line)) {
        std::stringstream ss(line);
        PlayerAppearance appearance;
        std::string token;

        skipColumns(ss, token);
        
        try {
            loadNumericCellIntoField(ss, token, appearance.gameId);
            loadNumericCellIntoField(ss, token, appearance.playerId);
            loadNumericCellIntoField(ss, token, appearance.clubId);

            skipColumns(ss, token, 6);
            
            loadNumericCellIntoField(ss, token, appearance.goals);
            loadNumericCellIntoField(ss, token, appearance.assists);
            loadNumericCellIntoField(ss, token, appearance.minutesPlayed);
        } catch (const std::invalid_argument (&e)) {
            std::cerr << "Invalid integer in data: " << token << std::endl;
            continue;
        }

        appearances.push_back(appearance);
    }

    return appearances;
}

#include "utils/database/Database.h"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <filesystem>

namespace fs = std::filesystem;

Database::Database(const std::string& dbPath) {
    bool isNewDatabase = !fileExists(dbPath);
    
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }

    if (isNewDatabase) {
        std::vector<std::string> tableNames {
            "appearances",
            "club_games",
            "clubs",
            "competitions",
            "game_events",
            "game_lineups",
            "games",
            "player_valuations",
            "players",
            "transfers"
        }; 

        downloadAndExtractDataset();
        executeSQLFile("../db/data.sql");

        for (const auto& tableName : tableNames) {
            std::string csvPath = "data/" + tableName + ".csv";
            loadCSVIntoTable(tableName, csvPath);
        }
    } else {
        std::cout << "Existing database used." << std::endl;
    }
}

Database::~Database() {
    if (db)
        sqlite3_close(db);
}

bool Database::fileExists(const std::string& dbPath) {
    struct stat buffer;
    return (stat(dbPath.c_str(), &buffer) == 0);
}

void Database::executeSQLFile(const std::string& filePath) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        std::cerr << "Failed to open SQL file: " << filePath << std::endl;
        return;
    }

    std::stringstream ss;
    std::string line;

    while (getline(file, line)) {
        ss << line << "\n";
    }

    char * errMsg;

    if (sqlite3_exec(db, ss.str().c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL schema execution error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Database schema initialized succesfully." << std::endl;
    }
}

sqlite3 * Database::getConnection() {
    return db;
}

std::string Database::sanitizeCSVValue(std::string value) {
    if (value.empty())
        return "NULL";

    value.erase(0, value.find_first_not_of(" \t\n\r"));
    value.erase(value.find_last_not_of(" \t\n\r") + 1);

    if (value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
        std::replace(value.begin(), value.end(), '\n', ' ');
    }

    std::string escapedValue;
    for (char c : value) {
        if (c == '\'') 
            escapedValue += "''";
        else 
            escapedValue += c;
    }

    return "'" + escapedValue + "'";
}

std::vector<std::string> Database::getSanitizedValues(std::ifstream& file, std::string& line) {
    std::vector<std::string> values;
    std::string value;
    std::string tempValue;
    bool inQuotes = false;

    while (true) {
        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];

            if (c == '"') {
                inQuotes = !inQuotes; 
            }

            if (c == ',' && !inQuotes) {
                values.push_back(sanitizeCSVValue(tempValue));
                tempValue.clear();
            } else {
                tempValue += c;
            }
        }

        if (inQuotes) { 
            std::string nextLine;
            if (!getline(file, nextLine)) 
                break;
            tempValue += "\n" + nextLine;
            line = nextLine;
        } else {
            break; 
        }
    }

    values.push_back(sanitizeCSVValue(tempValue));

    return values;
}

void Database::downloadAndExtractDataset() {
    std::string datasetPath = "data.zip";

    if (!fileExists(datasetPath)) {
        std::cout << "Downloading dataset from Kaggle..." << std::endl;
        std::system("curl -L -o data.zip "
                    "https://www.kaggle.com/api/v1/datasets/download/davidcariboo/player-scores");
    }

    if (!fs::exists("data")) {
        std::cout << "Extracting dataset..." << std::endl;
        std::system("unzip -o data.zip -d data");
    }
}


void Database::loadCSVIntoTable(const std::string& tableName, const std::string& csvPath) {
    std::ifstream file(csvPath);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file: " << csvPath << std::endl;
        return;
    }

    std::string line;
    std::stringstream sqlBatch;
    sqlBatch << "BEGIN TRANSACTION;\n";
    
    getline(file, line);

    while (getline(file, line)) {
        std::vector<std::string> values = getSanitizedValues(file, line);

        sqlBatch << "INSERT INTO " + tableName + " VALUES(";

        for (size_t i = 0; i < values.size(); i++) {
            sqlBatch << values[i];
            if (i < values.size() - 1) sqlBatch << ",";
        }
        sqlBatch << ");\n";
    }

    sqlBatch << "COMMIT;\n";

    char * errMsg;
    if (sqlite3_exec(db, sqlBatch.str().c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Data from " << csvPath << " imported into " << tableName << std::endl;
    }
}

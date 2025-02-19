#include "utils/Database.h"
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

Database::Database(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }
}

Database::~Database() {
    if (db)
        sqlite3_close(db);
}

const char * Database::getTableCommands() {
    return R"(
        CREATE TABLE IF NOT EXISTS appearances (
            appearance_id TEXT, 
            game_id INT, 
            player_id INT, 
            player_club_id INT,
            player_current_club_id INT, 
            date TEXT, 
            player_name TEXT,
            competition_id TEXT, 
            yellow_cards INT, 
            red_cards INT,
            goals INT, 
            assists INT, 
            minutes_played INT
        );
        CREATE TABLE IF NOT EXISTS games (
            game_id INT, 
            club_id INT, 
            own_goals INT, 
            own_position TEXT,
            own_manager_name TEXT, 
            opponent_id INT, 
            opponent_goals INT,
            opponent_position TEXT, 
            opponent_manager_name TEXT,
            hosting TEXT, 
            is_win INT
        );
        CREATE INDEX idx_appearances_game ON appearances(game_id);
        CREATE INDEX idx_appearances_player ON appearances(player_id);
    )";
}

void Database::initializeTables() {
    const char * tableCommands = getTableCommands();
    char* errMsg;

    if (sqlite3_exec(db, tableCommands, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Tables initialized" << std::endl;
    }
}

void Database::loadCSV(const std::string& tableName, const std::string& csvPath) {
    std::ifstream file(csvPath);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file: " << csvPath << std::endl;
        return;
    }

    std::string line;
    char * errMsg;
    getline(file, line);

    if (sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return;
    }

    while (getline(file, line)) {
        std::stringstream ss(line);
        std::vector<std::string> values;
        std::string value;

        while (getline(ss, value, ',')) {
            size_t pos = 0;

            while ((pos = value.find("'", pos)) != std::string::npos) {
                value.insert(pos, "'");
                pos += 2;
            }

            values.push_back(value);
        }

        std::string sql = "INSERT INTO " + tableName + " VALUES(";

        for (size_t i = 0; i < values.size(); i++) {
            sql += "'" + values[i] + "'";
            if (i < values.size() - 1)
                sql += ",";
        }
        sql += ");";

        if (sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }

    if (sqlite3_exec(db, "COMMIT;", 0, 0, &errMsg) != SQLITE_OK) {
        std:: cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    std::cout << "Data from " << csvPath << " imported into " << tableName << std::endl;
}

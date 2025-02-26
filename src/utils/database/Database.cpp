#include "utils/database/Database.h"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <filesystem>
#include <ctime>
#include <cstdlib>
#include <regex>

namespace fs = std::filesystem;

Database::Database(const std::string& dbPath) {    
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
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

void Database::loadDataIntoDatabase(bool updateDataset) {
    std::vector<std::string> tableNames {
        "appearances", "club_games", "clubs", "competitions",
        "game_events", "game_lineups", "games", "player_valuations",
        "players", "transfers"
    };

    downloadAndExtractDataset(updateDataset);

    for (const auto& tableName : tableNames) {
        std::string csvPath = "data/" + tableName + ".csv";
        loadCSVIntoTable(tableName, csvPath);
    }

    setLastUpdateTimestamp();
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
        std::cout << filePath << " initialized successfully." << std::endl;
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

void Database::downloadAndExtractDataset(bool updateDataset) {
    std::string datasetPath = "player-scores.zip";

    if (updateDataset || !fileExists(datasetPath)) {
        int result = std::system("kaggle datasets download -d davidcariboo/player-scores -p ./ 2> kaggle_error.log");

        if (result != 0) {
            std::cerr << "Kaggle API error. Check kaggle_error.log for details." << std::endl;
            return;
        }
    }

    if (updateDataset || !fs::exists("data")) {
        std::system("unzip -o player-scores.zip -d data");
    }
}

std::string Database::join(const std::vector<std::string>& values, const std::string& delimiter) {
    std::ostringstream result;
    for (size_t i = 0; i < values.size(); ++i) {
        result << values[i];
        if (i < values.size() - 1) {
            result << delimiter;
        }
    }
    return result.str();
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
    std::vector<std::string> columns = getSanitizedValues(file, line);

    if (columns.empty()) {
        std::cerr << "Error: Could not extract columns from CSV file: " << csvPath << std::endl;
        return;
    }

    std::string columnNames = "(" + join(columns, ", ") + ")";
    sqlBatch << "INSERT OR REPLACE INTO " + tableName + " " + columnNames + " VALUES ";

    bool firstRow = true;
    while (getline(file, line)) {
        std::vector<std::string> values = getSanitizedValues(file, line);

        if (!firstRow) {
            sqlBatch << ",";
        }
        sqlBatch << "(";
        for (size_t i = 0; i < values.size(); i++) {
            sqlBatch << values[i];
            if (i < values.size() - 1) sqlBatch << ",";
        }
        sqlBatch << ")";
        firstRow = false;
    }

    sqlBatch << ";\nCOMMIT;\n";

    char *errMsg;
    if (sqlite3_exec(db, sqlBatch.str().c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Data from " << csvPath << " imported into " << tableName << std::endl;
    }
}

time_t Database::getLastUpdateTimestamp() {
    const char* query = "SELECT value FROM metadata WHERE key = 'last_updated';";
    sqlite3_stmt* stmt;
    time_t lastUpdated = 0;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            lastUpdated = std::stol(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
    }
    sqlite3_finalize(stmt);
    return lastUpdated;
}

void Database::setLastUpdateTimestamp() {
    time_t now = time(0);
    std::string query = "INSERT INTO metadata (key, value) VALUES ('last_updated', '" + std::to_string(now) + 
                        "') ON CONFLICT(key) DO UPDATE SET value = '" + std::to_string(now) + "';";

    char *errMsg;
    if (sqlite3_exec(db, query.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to update last updated timestamp: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

void Database::updateDatasetIfNeeded() {
    std::string kaggleUsername = getMetadataValue("KAGGLE_USERNAME");
    std::string kaggleKey = getMetadataValue("KAGGLE_KEY");

    if (kaggleUsername.empty() || kaggleKey.empty()) {
        return;
    }

    setenv("KAGGLE_USERNAME", kaggleUsername.c_str(), 1);
    setenv("KAGGLE_KEY", kaggleKey.c_str(), 1);

    if (!fetchKaggleDatasetList()) {
        std::cerr << "Failed to fetch dataset metadata." << std::endl;
        return;
    }

    time_t kaggleUpdatedTime = extractLastUpdatedTimestamp();

    if (kaggleUpdatedTime == 0) {
        std::cerr << "Failed to extract last updated timestamp." << std::endl;
        return;
    }

    compareAndUpdateDataset(kaggleUpdatedTime);
}

std::string Database::getMetadataValue(const std::string& key) {
    std::string query = "SELECT value FROM metadata WHERE key = ?";
    sqlite3_stmt* stmt;
    std::string value;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            value = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
    }

    sqlite3_finalize(stmt);
    return value;
}

void Database::setMetadataValue(const std::string& key, const std::string& value) {
    std::string query = "INSERT INTO metadata (key, value) VALUES (?, ?) "
                        "ON CONFLICT(key) DO UPDATE SET value = excluded.value";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to set metadata value for key: " << key << std::endl;
        }
    }

    sqlite3_finalize(stmt);
}

bool Database::fetchKaggleDatasetList() {
    int result = std::system("kaggle datasets list --search 'davidcariboo/player-scores' --sort-by updated > dataset-list.txt");
    return result == 0;
}

time_t Database::extractLastUpdatedTimestamp() {
    std::ifstream file("dataset-list.txt");

    if (!file.is_open()) {
        std::cerr << "Failed to read dataset metadata file." << std::endl;
        return 0;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (line.find("davidcariboo/player-scores") != std::string::npos) {
            break;
        }
    }

    file.close();

    std::regex pattern(R"(\s(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}))");
    std::smatch match;

    if (!std::regex_search(line, match, pattern)) {
        std::cerr << "Failed to extract last updated timestamp from: " << line << std::endl;
        return 0;
    }

    std::string lastUpdatedStr = match[1].str();
    struct tm tm = {};

    if (strptime(lastUpdatedStr.c_str(), "%Y-%m-%d %H:%M:%S", &tm) == nullptr) {
        std::cerr << "Failed to parse timestamp: " << lastUpdatedStr << std::endl;
        return 0;
    }

    return mktime(&tm);
}

void Database::compareAndUpdateDataset(time_t kaggleUpdatedTime) {
    time_t lastUpdated = getLastUpdateTimestamp();

    if (kaggleUpdatedTime > lastUpdated) {
        std::cout << "New dataset available." << std::endl;
        loadDataIntoDatabase(true);
    } else {
        std::cout << "Dataset is already up-to-date." << std::endl;
    }
}

std::string Database::getKaggleUsername() {
    return getMetadataValue("KAGGLE_USERNAME");
}

std::string Database::getKaggleKey() {
    return getMetadataValue("KAGGLE_KEY");
}

void Database::setKaggleCredentials(const std::string& username, const std::string& key) {
    setMetadataValue("KAGGLE_USERNAME", username);
    setMetadataValue("KAGGLE_KEY", key);
}

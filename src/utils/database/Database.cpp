#include "utils/database/Database.h"
#include "utils/KaggleAPI.h"
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
#include <curl/curl.h>

namespace fs = std::filesystem;

size_t Database::WriteDataCallback(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

Database::Database(const std::string& path) : dbPath(path), kaggleClient(nullptr) {
    newDatabase = !fileExists(dbPath);
    
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }
}

void Database::initialize(std::function<void(const std::string&, int)> progressCallback) {
    newDatabase = !fileExists(dbPath);
    
    if (!newDatabase && !isDatabaseInitialized()) {
        newDatabase = true;
    }
    
    if (newDatabase) {
        progressCallback("Creating database schema", 15);
        executeSQLFile("../db/data.sql");
        executeSQLFile("../db/user.sql");
        loadDataIntoDatabase(false, progressCallback);
    } else {
        progressCallback("Checking for dataset updates", 15);
        updateDatasetIfNeeded(progressCallback);
    }
}

bool Database::isDatabaseInitialized() {
    std::vector<std::string> requiredTables = {
        "appearances", "club_games", "clubs", "competitions",
        "game_events", "game_lineups", "games", "player_valuations",
        "players", "transfers", "metadata"
    };
    
    for (const auto& table : requiredTables) {
        if (!tableExists(table)) {
            return false;
        }
        
        if (table != "metadata" && !tableHasData(table)) {
            return false;
        }
    }
    
    if (!metadataHasEntry("last_updated")) {
        return false;
    }
    
    return true;
}

bool Database::tableExists(const std::string& tableName) {
    std::string query = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    sqlite3_stmt* stmt;
    bool exists = false;
    
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, tableName.c_str(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = true;
        }
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

bool Database::tableHasData(const std::string& tableName) {
    if (tableName == "metadata") {
        return true;
    }
    
    std::string query = "SELECT COUNT(*) FROM " + tableName + ";";
    sqlite3_stmt* stmt;
    bool hasData = false;
    
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            hasData = (count > 0);
        }
    } else {
        std::cerr << "Error checking data in table '" << tableName << "': " 
                  << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    return hasData;
}

bool Database::metadataHasEntry(const std::string& key) {
    std::string value = getMetadataValue(key);
    return !value.empty();
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
    
    delete kaggleClient;
    kaggleClient = nullptr;
}

bool Database::fileExists(const std::string& dbPath) {
    struct stat buffer;
    return (stat(dbPath.c_str(), &buffer) == 0);
}

void Database::loadDataIntoDatabase(bool updateDataset, std::function<void(const std::string&, int)> progressCallback) {
    std::vector<std::string> tableNames {
        "appearances", "club_games", "clubs", "competitions",
        "game_events", "game_lineups", "games", "player_valuations",
        "players", "transfers"
    };

    progressCallback("Downloading dataset", 20);
    bool downloadSuccess = downloadAndExtractDataset(updateDataset, progressCallback);
    
    if (!downloadSuccess) {
        std::cerr << "Failed to download or extract dataset. Database initialization aborted." << std::endl;
        return;
    }
    
    if (!fs::exists("data") || !fs::exists("data/players.csv")) {
        std::cerr << "Data directory or essential CSV files missing. Database initialization aborted." << std::endl;
        return;
    }

    int baseProgress = 30;
    int progressPerTable = 40 / static_cast<int>(tableNames.size());
    
    for (size_t i = 0; i < tableNames.size(); i++) {
        std::string tableName = tableNames[i];
        std::string csvPath = "data/" + tableName + ".csv";
        
        if (!fileExists(csvPath)) {
            std::cerr << "Warning: CSV file not found: " << csvPath << ". Skipping this table." << std::endl;
            continue;
        }
        
        progressCallback("Loading " + tableName + " data", baseProgress + static_cast<int>(i) * progressPerTable);
        loadCSVIntoTable(tableName, csvPath);
    }

    progressCallback("Finalizing database setup", 70);
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

bool Database::downloadAndExtractDataset(bool updateDataset, std::function<void(const std::string&, int)> progressCallback) {
    std::string datasetPath = "player-scores.zip";

    if (updateDataset || !fileExists(datasetPath)) {
        progressCallback("Downloading dataset from Kaggle", 20);
        
        KaggleAPIClient client;
        
        if (!client.downloadDataset("davidcariboo/player-scores", datasetPath)) {
            std::cerr << "Failed to download dataset." << std::endl;
            return false;
        }
    }

    if (updateDataset || !fs::exists("data")) {
        progressCallback("Extracting dataset files", 25);
        
        if (!fs::exists("data")) {
            fs::create_directory("data");
        }
        
        try {
            #ifdef _WIN32
                std::string cmd = "powershell -Command \"Expand-Archive -Path player-scores.zip -DestinationPath data -Force\"";
                int result = std::system(cmd.c_str());
                if (result != 0) {
                    std::cerr << "Error extracting zip file with PowerShell. Return code: " << result << std::endl;
                    return false;
                }
            #else
                int result = std::system("unzip -o player-scores.zip -d data");
                if (result != 0) {
                    std::cerr << "Error extracting zip file with unzip. Return code: " << result << std::endl;
                    return false;
                }
            #endif
        } catch (const std::exception& e) {
            std::cerr << "Exception while extracting dataset: " << e.what() << std::endl;
            return false;
        }
    }
    
    if (!fs::exists("data/players.csv")) {
        std::cerr << "Dataset extraction failed. Could not find CSV files in data directory." << std::endl;
        return false;
    }
    
    return true;
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

    std::string headerLine;
    getline(file, headerLine);
    std::vector<std::string> columns = getSanitizedValues(file, headerLine);
    if (columns.empty()) {
        std::cerr << "Error: Could not extract columns from CSV file: " << csvPath << std::endl;
        return;
    }

    std::string query = buildInsertQuery(tableName, columns, file);
    executeCSVImportQuery(query, csvPath, tableName);
}

std::string Database::buildInsertQuery(const std::string& tableName, 
                                      const std::vector<std::string>& columns, 
                                      std::ifstream& file) {
    std::stringstream sqlBatch;
    sqlBatch << "BEGIN TRANSACTION;\n";

    std::string columnNames = "(" + join(columns, ", ") + ")";
    sqlBatch << "INSERT OR REPLACE INTO " + tableName + " " + columnNames + " VALUES ";

    bool firstRow = true;
    appendCSVRowsToQuery(file, sqlBatch, firstRow);
    
    sqlBatch << ";\nCOMMIT;\n";
    return sqlBatch.str();
}

void Database::appendCSVRowsToQuery(std::ifstream& file, 
                                   std::stringstream& sqlBatch,
                                   bool& firstRow) {
    std::string line;
    while (getline(file, line)) {
        std::vector<std::string> values = getSanitizedValues(file, line);
        if (values.empty()) {
            continue;
        }

        if (!firstRow) {
            sqlBatch << ",";
        }
        
        appendRowValuesToQuery(values, sqlBatch);
        firstRow = false;
    }
}

void Database::appendRowValuesToQuery(const std::vector<std::string>& values, 
                                     std::stringstream& sqlBatch) {
    sqlBatch << "(";
    for (size_t i = 0; i < values.size(); i++) {
        sqlBatch << values[i];
        if (i < values.size() - 1) {
            sqlBatch << ",";
        }
    }
    sqlBatch << ")";
}

void Database::executeCSVImportQuery(const std::string& query, 
                                    const std::string& csvPath,
                                    const std::string& tableName) {
    char *errMsg;
    if (sqlite3_exec(db, query.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
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

std::string Database::formatTimestamp(time_t timestamp) {
    struct tm* timeinfo = localtime(&timestamp);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return std::string(buffer);
}

void Database::updateDatasetIfNeeded(std::function<void(const std::string&, int)> progressCallback) {
    std::string kaggleUsername = getMetadataValue("KAGGLE_USERNAME");
    std::string kaggleKey = getMetadataValue("KAGGLE_KEY");

    if (kaggleUsername.empty() || kaggleKey.empty()) {
        progressCallback("Kaggle credentials not set. Skipping update check.", 70);
        return;
    }

    progressCallback("Checking for dataset updates", 25);
    
    time_t kaggleUpdatedTime = getKaggleDatasetLastUpdated();

    if (kaggleUpdatedTime == 0) {
        std::cerr << "Failed to get dataset last updated timestamp." << std::endl;
        return;
    }

    progressCallback("Comparing dataset versions", 30);
    
    compareAndUpdateDataset(kaggleUpdatedTime, progressCallback);
}

std::string Database::getMetadataValue(const std::string& key) {
    if (!tableExists("metadata")) {
        return "";
    }
    
    std::string query = "SELECT value FROM metadata WHERE key = ?;";
    sqlite3_stmt* stmt;
    std::string value;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* storedValue = sqlite3_column_text(stmt, 0);
            value = storedValue != nullptr ? reinterpret_cast<const char*>(storedValue) : "";
        }
    }

    sqlite3_finalize(stmt);
    return value;
}

void Database::setMetadataValue(const std::string& key, const std::string& value) {
    if (!tableExists("metadata")) {
        char* errMsg;
        if (sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS metadata (key TEXT PRIMARY KEY, value TEXT);", 
                         NULL, NULL, &errMsg) != SQLITE_OK) {
            std::cerr << "Failed to create metadata table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return;
        }
    }
    
    std::string query = "INSERT INTO metadata (key, value) VALUES (?, ?) "
                        "ON CONFLICT(key) DO UPDATE SET value = excluded.value;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to set metadata value for key: " << key << std::endl;
        }
    }

    sqlite3_finalize(stmt);
}

time_t Database::getKaggleDatasetLastUpdated() {
    std::string kaggleUsername = getMetadataValue("KAGGLE_USERNAME");
    std::string kaggleKey = getMetadataValue("KAGGLE_KEY");
    
    if (kaggleUsername.empty() || kaggleKey.empty()) {
        std::cerr << "Kaggle credentials not set. Cannot check for dataset updates." << std::endl;
        return 0;
    }
    
    if (!kaggleClient || kaggleClient->getUsername() != kaggleUsername || kaggleClient->getKey() != kaggleKey) {
        delete kaggleClient;
        kaggleClient = new KaggleAPIClient(kaggleUsername, kaggleKey);
    }
    
    return kaggleClient->getDatasetLastUpdated("davidcariboo/player-scores");
}

void Database::compareAndUpdateDataset(time_t kaggleUpdatedTime, std::function<void(const std::string&, int)> progressCallback) {
    time_t lastUpdated = getLastUpdateTimestamp();

    if (kaggleUpdatedTime > lastUpdated) {
        progressCallback("New dataset available, updating", 35);
        loadDataIntoDatabase(true, progressCallback);
    } else {
        progressCallback("Dataset is already up-to-date", 70);
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

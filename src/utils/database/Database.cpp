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
    return fwrite(ptr, size, nmemb, stream);
}

Database::Database(std::string_view dbPath) 
    : m_dbPath(dbPath) {
    m_newDatabase = !fileExists(dbPath);
    
    if (sqlite3_open(m_dbPath.c_str(), &m_db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(m_db) << std::endl;
    }
}

Database::~Database() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

sqlite3* Database::getConnection() const {
    return m_db;
}

bool Database::fileExists(std::string_view filePath) const {
    try {
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    } catch (const fs::filesystem_error&) {
        struct stat buffer;
        return (stat(std::string(filePath).c_str(), &buffer) == 0);
    }
}

void Database::initialize(ProgressCallback progressCallback) {
    m_newDatabase = !fileExists(m_dbPath) || !isDatabaseInitialized();
    
    if (m_newDatabase) {
        if (progressCallback) {
            progressCallback("Creating database schema", 15);
        }
        executeSQLFile("../db/data.sql");
        executeSQLFile("../db/user.sql");
        loadDataIntoDatabase(false, progressCallback);
    } else {
        if (progressCallback) {
            progressCallback("Checking for dataset updates", 15);
        }
        updateDatasetIfNeeded(progressCallback);
    }
}

bool Database::isDatabaseInitialized() const {
    const std::vector<std::string> requiredTables = {
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
    
    return metadataHasEntry("last_updated");
}

bool Database::tableExists(std::string_view tableName) const {
    sqlite3_stmt* stmt = nullptr;
    bool exists = false;
    const std::string query = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    
    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, tableName.data(), static_cast<int>(tableName.size()), SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = true;
        }
    }
    
    sqlite3_finalize(stmt);
    return exists;
}

bool Database::tableHasData(std::string_view tableName) const {
    if (tableName == "metadata") {
        return true;
    }
    
    std::string query = "SELECT COUNT(*) FROM " + std::string(tableName) + ";";
    sqlite3_stmt* stmt = nullptr;
    bool hasData = false;
    
    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            hasData = (sqlite3_column_int(stmt, 0) > 0);
        }
    }
    
    sqlite3_finalize(stmt);
    return hasData;
}

bool Database::metadataHasEntry(std::string_view key) const {
    return !getMetadataValue(key).empty();
}

bool Database::isNewDatabase() const {
    return m_newDatabase;
}

void Database::executeSQLFile(std::string_view filePath) {
    std::ifstream file(filePath.data());

    if (!file.is_open()) {
        std::cerr << "Failed to open SQL file: " << filePath << std::endl;
        return;
    }

    std::stringstream ss;
    std::string line;

    while (std::getline(file, line)) {
        ss << line << "\n";
    }

    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, ss.str().c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL schema execution error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

std::string Database::sanitizeCSVValue(std::string_view value) const {
    if (value.empty()) {
        return "NULL";
    }

    std::string result(value);
    
    auto startPos = result.find_first_not_of(" \t\n\r");
    auto endPos = result.find_last_not_of(" \t\n\r");
    
    if (startPos == std::string::npos) {
        return "NULL";
    }
    
    result = result.substr(startPos, endPos - startPos + 1);

    if (result.front() == '"' && result.back() == '"' && result.size() > 1) {
        result = result.substr(1, result.size() - 2);
        std::replace(result.begin(), result.end(), '\n', ' ');
    }

    std::string escapedValue;
    for (char c : result) {
        if (c == '\'') {
            escapedValue += "''";
        } else {
            escapedValue += c;
        }
    }

    return "'" + escapedValue + "'";
}

std::vector<std::string> Database::getSanitizedValues(std::ifstream& file, std::string& line) const {
    std::vector<std::string> values;
    std::string tempValue;
    bool inQuotes = false;

    while (true) {
        for (char c : line) {
            if (c == '"') {
                inQuotes = !inQuotes; 
            } else if (c == ',' && !inQuotes) {
                values.push_back(sanitizeCSVValue(tempValue));
                tempValue.clear();
            } else {
                tempValue += c;
            }
        }

        if (inQuotes) { 
            std::string nextLine;
            if (!std::getline(file, nextLine)) {
                break;
            }
            tempValue += "\n" + nextLine;
            line = nextLine;
        } else {
            break; 
        }
    }

    values.push_back(sanitizeCSVValue(tempValue));
    return values;
}

bool Database::downloadAndExtractDataset(bool updateDataset, ProgressCallback progressCallback) {
    const std::string datasetPath = "player-scores.zip";

    if (updateDataset || !fileExists(datasetPath)) {
        if (progressCallback) {
            progressCallback("Downloading dataset from Kaggle", 20);
        }
        
        KaggleAPIClient client;
        
        if (!client.downloadDataset("davidcariboo/player-scores", datasetPath)) {
            std::cerr << "Failed to download dataset." << std::endl;
            return false;
        }
    }

    if (updateDataset || !fs::exists("data")) {
        if (progressCallback) {
            progressCallback("Extracting dataset files", 25);
        }
        
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

std::string Database::joinStrings(std::span<const std::string> values, std::string_view delimiter) const {
    std::ostringstream result;
    for (size_t i = 0; i < values.size(); ++i) {
        result << values[i];
        if (i < values.size() - 1) {
            result << delimiter;
        }
    }
    return result.str();
}

void Database::loadDataIntoDatabase(bool updateDataset, ProgressCallback progressCallback) {
    std::vector<std::string> tableNames {
        "appearances", "club_games", "clubs", "competitions",
        "game_events", "game_lineups", "games", "player_valuations",
        "players", "transfers"
    };

    if (progressCallback) {
        progressCallback("Downloading dataset", 20);
    }
    
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
        
        if (progressCallback) {
            progressCallback("Loading " + tableName + " data", baseProgress + static_cast<int>(i) * progressPerTable);
        }
        loadCSVIntoTable(tableName, csvPath);
    }

    if (progressCallback) {
        progressCallback("Finalizing database setup", 70);
    }
    setLastUpdateTimestamp();
}

std::string Database::buildInsertQuery(std::string_view tableName, 
                                     std::span<const std::string> columns, 
                                     std::ifstream& file) const {
    std::stringstream sqlBatch;
    sqlBatch << "BEGIN TRANSACTION;\n";

    std::string columnList = "(" + joinStrings(columns, ", ") + ")";
    sqlBatch << "INSERT OR REPLACE INTO " << tableName << " " << columnList << " VALUES ";

    bool firstRow = true;
    appendCSVRowsToQuery(file, sqlBatch, firstRow);
    
    sqlBatch << ";\nCOMMIT;\n";
    return sqlBatch.str();
}

void Database::appendCSVRowsToQuery(std::ifstream& file, std::stringstream& sqlBatch, bool& firstRow) const {
    std::string line;
    while (std::getline(file, line)) {
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

void Database::appendRowValuesToQuery(std::span<const std::string> values, std::stringstream& sqlBatch) const {
    sqlBatch << "(";
    for (size_t i = 0; i < values.size(); i++) {
        sqlBatch << values[i];
        if (i < values.size() - 1) {
            sqlBatch << ",";
        }
    }
    sqlBatch << ")";
}

void Database::executeCSVImportQuery(std::string_view query, std::string_view csvPath, std::string_view tableName) {
    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, query.data(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Data from " << csvPath << " imported into " << tableName << std::endl;
    }
}

time_t Database::getLastUpdateTimestamp() const {
    const char* query = "SELECT value FROM metadata WHERE key = 'last_updated';";
    sqlite3_stmt* stmt = nullptr;
    time_t lastUpdated = 0;

    if (sqlite3_prepare_v2(m_db, query, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            lastUpdated = std::stol(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
    }
    sqlite3_finalize(stmt);
    return lastUpdated;
}

void Database::setLastUpdateTimestamp() {
    time_t now = time(nullptr);
    std::string query = "INSERT INTO metadata (key, value) VALUES ('last_updated', '" + std::to_string(now) + 
                        "') ON CONFLICT(key) DO UPDATE SET value = '" + std::to_string(now) + "';";

    char* errMsg = nullptr;
    if (sqlite3_exec(m_db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to update last updated timestamp: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

std::string Database::formatTimestamp(time_t timestamp) const {
    struct tm timeinfo;
    char buffer[80];
    
#ifdef _WIN32
    localtime_s(&timeinfo, &timestamp);
#else
    localtime_r(&timestamp, &timeinfo);
#endif
    
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

void Database::updateDatasetIfNeeded(ProgressCallback progressCallback) {
    std::string kaggleUsername = getMetadataValue("KAGGLE_USERNAME");
    std::string kaggleKey = getMetadataValue("KAGGLE_KEY");

    if (kaggleUsername.empty() || kaggleKey.empty()) {
        if (progressCallback) {
            progressCallback("Kaggle credentials not set. Skipping update check.", 70);
        }
        return;
    }

    if (progressCallback) {
        progressCallback("Checking for dataset updates", 25);
    }
    
    time_t kaggleUpdatedTime = getKaggleDatasetLastUpdated();

    if (kaggleUpdatedTime == 0) {
        std::cerr << "Failed to get dataset last updated timestamp." << std::endl;
        return;
    }

    if (progressCallback) {
        progressCallback("Comparing dataset versions", 30);
    }
    
    compareAndUpdateDataset(kaggleUpdatedTime, progressCallback);
}

std::string Database::getMetadataValue(std::string_view key) const {
    if (!tableExists("metadata")) {
        return "";
    }
    
    sqlite3_stmt* stmt = nullptr;
    std::string value;
    std::string query = "SELECT value FROM metadata WHERE key = ?;";

    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.data(), static_cast<int>(key.size()), SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* storedValue = sqlite3_column_text(stmt, 0);
            value = storedValue != nullptr ? reinterpret_cast<const char*>(storedValue) : "";
        }
    }

    sqlite3_finalize(stmt);
    return value;
}

void Database::setMetadataValue(std::string_view key, std::string_view value) {
    if (!tableExists("metadata")) {
        char* errMsg = nullptr;
        if (sqlite3_exec(m_db, "CREATE TABLE IF NOT EXISTS metadata (key TEXT PRIMARY KEY, value TEXT);", 
                        nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "Failed to create metadata table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return;
        }
    }
    
    sqlite3_stmt* stmt = nullptr;
    std::string query = "INSERT INTO metadata (key, value) VALUES (?, ?) "
                        "ON CONFLICT(key) DO UPDATE SET value = excluded.value;";
    
    if (sqlite3_prepare_v2(m_db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, key.data(), static_cast<int>(key.size()), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to set metadata value for key: " << key << std::endl;
        }
    }

    sqlite3_finalize(stmt);
}

time_t Database::getKaggleDatasetLastUpdated() const {
    std::string kaggleUsername = getMetadataValue("KAGGLE_USERNAME");
    std::string kaggleKey = getMetadataValue("KAGGLE_KEY");
    
    if (kaggleUsername.empty() || kaggleKey.empty()) {
        std::cerr << "Kaggle credentials not set. Cannot check for dataset updates." << std::endl;
        return 0;
    }
    
    if (!m_kaggleClient || m_kaggleClient->getUsername() != kaggleUsername || m_kaggleClient->getKey() != kaggleKey) {
        m_kaggleClient = std::make_unique<KaggleAPIClient>(kaggleUsername, kaggleKey);
    }
    
    return m_kaggleClient->getDatasetLastUpdated("davidcariboo/player-scores");
}

void Database::compareAndUpdateDataset(time_t kaggleUpdatedTime, ProgressCallback progressCallback) {
    time_t lastUpdated = getLastUpdateTimestamp();

    if (kaggleUpdatedTime > lastUpdated) {
        if (progressCallback) {
            progressCallback("New dataset available, updating", 35);
        }
        loadDataIntoDatabase(true, progressCallback);
    } else {
        if (progressCallback) {
            progressCallback("Dataset is already up-to-date", 70);
        }
    }
}

std::string Database::getKaggleUsername() const {
    return getMetadataValue("KAGGLE_USERNAME");
}

std::string Database::getKaggleKey() const {
    return getMetadataValue("KAGGLE_KEY");
}

void Database::setKaggleCredentials(std::string_view username, std::string_view key) {
    setMetadataValue("KAGGLE_USERNAME", username);
    setMetadataValue("KAGGLE_KEY", key);
}

void Database::loadCSVIntoTable(std::string_view tableName, std::string_view csvPath) {
    std::ifstream file(csvPath.data());
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file: " << csvPath << std::endl;
        return;
    }

    std::string headerLine;
    std::getline(file, headerLine);
    std::vector<std::string> columns = getSanitizedValues(file, headerLine);
    
    if (columns.empty()) {
        std::cerr << "Error: Could not extract columns from CSV file: " << csvPath << std::endl;
        return;
    }

    std::string query = buildInsertQuery(tableName, columns, file);
    executeCSVImportQuery(query, csvPath, tableName);
}

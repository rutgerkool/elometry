#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include <functional>
#ifdef _WIN32
#include <stdlib.h>
#include <iomanip>
#include <sstream>
#endif

class KaggleAPIClient;

typedef int PlayerId;

class Database {
    public:
        Database(const std::string& dbPath);
        ~Database();

        sqlite3 * getConnection();

        std::string getKaggleUsername();
        std::string getKaggleKey();
        void setKaggleCredentials(const std::string& username, const std::string& key);
        void loadCSVIntoTable(const std::string& tableName, const std::string& csvPath);
        void executeSQLFile(const std::string& filePath);
        bool fileExists(const std::string& dbPath);
        void loadDataIntoDatabase(bool updateDataset = false, std::function<void(const std::string&, int)> progressCallback = nullptr);
        void updateDatasetIfNeeded(std::function<void(const std::string&, int)> progressCallback = nullptr);
        bool isNewDatabase() const { return newDatabase; }
        void initialize(std::function<void(const std::string&, int)> progressCallback = nullptr);

    private:
        sqlite3 * db = nullptr;
        std::string dbPath;
        bool newDatabase = false;
        KaggleAPIClient* kaggleClient = nullptr;

        std::string sanitizeCSVValue(std::string value);
        std::string join(const std::vector<std::string>& values, const std::string& delimiter);
        std::vector<std::string> getSanitizedValues(std::ifstream& file, std::string& line);
        void setLastUpdateTimestamp();
        time_t getLastUpdateTimestamp();
        std::string getMetadataValue(const std::string& key);
        void setMetadataValue(const std::string& key, const std::string& value);
        bool downloadAndExtractDataset(bool updateDataset = false, std::function<void(const std::string&, int)> progressCallback = nullptr);
        time_t getKaggleDatasetLastUpdated();
        std::string formatTimestamp(time_t timestamp);
        void compareAndUpdateDataset(time_t kaggleUpdatedTime, std::function<void(const std::string&, int)> progressCallback = nullptr);
        
        bool isDatabaseInitialized();
        bool tableExists(const std::string& tableName);
        bool tableHasData(const std::string& tableName);
        bool metadataHasEntry(const std::string& key);

        std::string buildInsertQuery(const std::string& tableName, const std::vector<std::string>& columns, std::ifstream& file);
        void appendCSVRowsToQuery(std::ifstream& file, std::stringstream& sqlBatch, bool& firstRow);
        void appendRowValuesToQuery(const std::vector<std::string>& values, std::stringstream& sqlBatch);
        void executeCSVImportQuery(const std::string& query, const std::string& csvPath, const std::string& tableName);
        
        static size_t WriteDataCallback(void* ptr, size_t size, size_t nmemb, FILE* stream);
};

#endif

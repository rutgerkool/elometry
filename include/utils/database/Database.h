#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include <functional>

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

        std::string sanitizeCSVValue(std::string value);
        std::string join(const std::vector<std::string>& values, const std::string& delimiter);
        std::vector<std::string> getSanitizedValues(std::ifstream& file, std::string& line);
        void setLastUpdateTimestamp();
        time_t getLastUpdateTimestamp();
        time_t extractLastUpdatedTimestamp();
        std::string getMetadataValue(const std::string& key);
        void setMetadataValue(const std::string& key, const std::string& value);
        void downloadAndExtractDataset(bool updateDataset = false, std::function<void(const std::string&, int)> progressCallback = nullptr);
        bool fetchKaggleDatasetList();
        void compareAndUpdateDataset(time_t kaggleUpdatedTime, std::function<void(const std::string&, int)> progressCallback = nullptr);
};

#endif

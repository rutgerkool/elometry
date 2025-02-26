#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>

typedef int PlayerId;

class Database {
    public:
        Database(const std::string& dbPath);
        ~Database();

        sqlite3 * getConnection();

        std::string getKaggleUsername();
        std::string getKaggleKey();
        void setKaggleCredentials(const std::string& username, const std::string& key);

    private:
        sqlite3 * db = nullptr;

        std::string sanitizeCSVValue(std::string value);
        std::string join(const std::vector<std::string>& values, const std::string& delimiter);
        std::vector<std::string> getSanitizedValues(std::ifstream& file, std::string& line);
        void setLastUpdateTimestamp();
        time_t getLastUpdateTimestamp();
        time_t extractLastUpdatedTimestamp();
        std::string getMetadataValue(const std::string& key);
        void setMetadataValue(const std::string& key, const std::string& value);
        void executeSQLFile(const std::string& filePath);
        void downloadAndExtractDataset(bool updateDataset = false);
        void loadCSVIntoTable(const std::string& tableName, const std::string& csvPath);
        void loadDataIntoDatabase(bool updateDataset = false);
        void updateDatasetIfNeeded();
        bool fetchKaggleDatasetList();
        bool fileExists(const std::string& dbPath);
        void compareAndUpdateDataset(time_t kaggleUpdatedTime);
};

#endif

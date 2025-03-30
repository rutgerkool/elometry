#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>
#include <functional>
#include <filesystem>
#include <span>
#include <optional>
#include <memory>

class KaggleAPIClient;

using ProgressCallback = std::function<void(const std::string&, int)>;
using PlayerId = int;

class Database {
public:
    explicit Database(std::string_view dbPath);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept = delete;
    Database& operator=(Database&&) noexcept = delete;

    [[nodiscard]] sqlite3* getConnection() const;

    [[nodiscard]] std::string getKaggleUsername() const;
    [[nodiscard]] std::string getKaggleKey() const;
    void setKaggleCredentials(std::string_view username, std::string_view key);
    void loadCSVIntoTable(std::string_view tableName, std::string_view csvPath);
    void executeSQLFile(std::string_view filePath);
    [[nodiscard]] bool isNewDatabase() const;
    void initialize(ProgressCallback progressCallback = nullptr);

private:
    struct MetadataRecord {
        std::string key;
        std::string value;
    };

    sqlite3* m_db{nullptr};
    std::string m_dbPath;
    bool m_newDatabase{false};
    mutable std::unique_ptr<KaggleAPIClient> m_kaggleClient;

    [[nodiscard]] bool fileExists(std::string_view filePath) const;
    [[nodiscard]] bool isDatabaseInitialized() const;
    [[nodiscard]] bool tableExists(std::string_view tableName) const;
    [[nodiscard]] bool tableHasData(std::string_view tableName) const;
    [[nodiscard]] bool metadataHasEntry(std::string_view key) const;

    void loadDataIntoDatabase(bool updateDataset, ProgressCallback progressCallback);
    void updateDatasetIfNeeded(ProgressCallback progressCallback);
    [[nodiscard]] bool downloadAndExtractDataset(bool updateDataset, ProgressCallback progressCallback);
    void setLastUpdateTimestamp();
    [[nodiscard]] time_t getLastUpdateTimestamp() const;
    [[nodiscard]] std::string getMetadataValue(std::string_view key) const;
    void setMetadataValue(std::string_view key, std::string_view value);
    [[nodiscard]] time_t getKaggleDatasetLastUpdated() const;
    [[nodiscard]] std::string formatTimestamp(time_t timestamp) const;
    void compareAndUpdateDataset(time_t kaggleUpdatedTime, ProgressCallback progressCallback);

    [[nodiscard]] std::vector<std::string> getSanitizedValues(std::ifstream& file, std::string& line) const;
    [[nodiscard]] std::string sanitizeCSVValue(std::string_view value) const;
    [[nodiscard]] std::string buildInsertQuery(std::string_view tableName, 
                                              std::span<const std::string> columns, 
                                              std::ifstream& file) const;
    void appendCSVRowsToQuery(std::ifstream& file, std::stringstream& sqlBatch, bool& firstRow) const;
    void appendRowValuesToQuery(std::span<const std::string> values, std::stringstream& sqlBatch) const;
    void executeCSVImportQuery(std::string_view query, std::string_view csvPath, std::string_view tableName);
    [[nodiscard]] std::string joinStrings(std::span<const std::string> values, std::string_view delimiter) const;

    static size_t WriteDataCallback(void* ptr, size_t size, size_t nmemb, FILE* stream);
};

#endif

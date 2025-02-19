#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include <sqlite3.h>

class Database {
    public:
        Database(const std::string& dbPath);
        ~Database();
        sqlite3 * getConnection();
        bool fileExists(const std::string& dbPath);

        void executeSQLFile(const std::string& filePath);
        std::string sanitizeCSVValue(std::string value);
        std::vector<std::string> getSanitizedValues(std::ifstream& file, std::string& line);
        void loadCSVIntoTable(const std::string& tableName, const std::string& csvPath);

    private:
        sqlite3 * db = nullptr;

        const char * getTableCommands();
};

#endif

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <sqlite3.h>

class Database {
    public:
        Database(const std::string& dbPath);
        ~Database();
        sqlite3 * getConnection();
        bool fileExists(const std::string& dbPath);

        void initializeTables();
        void loadCSV(const std::string& tableName, const std::string& csvPath);

    private:
        sqlite3 * db = nullptr;

        const char * getTableCommands();
};

#endif

#include <string>
#include <sqlite3.h>

class Database {
    public:
        Database(const std::string& dbPath);
        ~Database();

        void initializeTables();
        void loadCSV(const std::string& tableName, const std::string& csvPath);

    private:
        sqlite3 *db = nullptr;

        const char *getTableCommands();
};

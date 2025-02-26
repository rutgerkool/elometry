#include "gui/components/DataLoader.h"
#include <QThread>

DataLoader::DataLoader(RatingManager& rm, TeamManager& tm, Database& db, QObject* parent)
    : QObject(parent)
    , ratingManager(rm)
    , teamManager(tm)
    , database(db)
{
}

void DataLoader::loadData(const std::string& dbPath)
{
    emit progressUpdate("Connecting to database", 10);
    bool isNewDatabase = !database.fileExists(dbPath);
    QThread::msleep(500);
    
    if (isNewDatabase) {
        database.executeSQLFile("../db/data.sql");
        database.executeSQLFile("../db/user.sql");

        emit progressUpdate("Loading dataset into database", 25);
        database.loadDataIntoDatabase();
    } else {
        emit progressUpdate("Checking for dataset updates", 25);
        database.updateDatasetIfNeeded();
    }

    emit progressUpdate("Processing player ratings...", 75);
    ratingManager.loadAndProcessRatings();

    
    emit progressUpdate("Loading team data...", 90);
    teamManager.loadTeams();
    
    emit progressUpdate("Ready!", 100);
    
    emit loadingComplete();
}

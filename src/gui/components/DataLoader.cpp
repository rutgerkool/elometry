#include "gui/components/DataLoader.h"
#include <filesystem>

namespace fs = std::filesystem;

DataLoader::DataLoader(RatingManager& rm, TeamManager& tm, Database& db, QObject* parent)
    : QObject(parent)
    , ratingManager(rm)
    , teamManager(tm)
    , database(db)
{
}

void DataLoader::loadData()
{
    emit progressUpdate("Connecting to database", 10);
    
    if (!fs::exists("data")) {
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

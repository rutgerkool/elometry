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
    
    auto progressCallback = [this](const std::string& status, int progress) {
        emit progressUpdate(QString::fromStdString(status), progress);
    };
    
    if (database.isNewDatabase()) {
        database.initialize(progressCallback);
    } else {
        database.initialize(progressCallback);
    }

    emit progressUpdate("Processing player ratings", 75);
    ratingManager.loadAndProcessRatings();
    
    emit progressUpdate("Loading team data", 90);
    teamManager.loadTeams();
    
    emit progressUpdate("Ready!", 100);
    
    emit loadingComplete();
}

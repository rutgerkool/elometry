#include "gui/components/DataLoader.h"
#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/Database.h"

#include <QCoreApplication>
#include <QTimer>
#include <filesystem>

namespace fs = std::filesystem;

DataLoader::DataLoader(RatingManager& ratingManager, 
                     TeamManager& teamManager, 
                     Database& database, 
                     QObject* parent)
    : QObject(parent)
    , m_ratingManager(ratingManager)
    , m_teamManager(teamManager)
    , m_database(database)
{}

void DataLoader::loadData() {
    notifyProgress("Connecting to database", 10);
    initializeDatabase();
    
    loadRatingData();
    loadTeamData();
    
    notifyProgress("Ready!", 100);
    emit loadingComplete();
}

void DataLoader::initializeDatabase() {
    auto progressCallback = [this](const std::string& status, int progress) {
        emit progressUpdate(QString::fromStdString(status), progress);
    };
    
    m_database.initialize(progressCallback);
}

void DataLoader::loadRatingData() {
    notifyProgress("Processing player ratings", 75);
    m_ratingManager.loadAndProcessRatings();
}

void DataLoader::loadTeamData() {
    notifyProgress("Loading team data", 90);
    m_teamManager.loadTeams();
}

void DataLoader::notifyProgress(const QString& status, int progress) {
    emit progressUpdate(status, progress);
    QCoreApplication::processEvents();
}

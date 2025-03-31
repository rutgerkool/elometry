#include "gui/views/MainWindow.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QStyle>
#include <QStyleFactory>
#include "utils/database/Database.h"
#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/repositories/TeamRepository.h"

namespace {

    void applyApplicationStyle(QApplication& app) {
        app.setStyle(QStyleFactory::create("Fusion"));

        QFile styleFile(":/styles/styles.qss");
        if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
            app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
            styleFile.close();
        }

        app.setFont(QFont("Segoe UI", 10));
    }

    auto initializeServices(Database& database) {
        auto ratingManager = std::make_unique<RatingManager>(database);
        auto teamRepository = std::make_unique<TeamRepository>(database);
        auto playerRepository = std::make_unique<PlayerRepository>(database);
        
        auto teamManager = std::make_unique<TeamManager>(
            *teamRepository,
            *ratingManager,
            *playerRepository
        );
        
        return std::tuple{
            std::move(ratingManager),
            std::move(teamManager),
            std::move(teamRepository),
            std::move(playerRepository)
        };
    }

}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    applyApplicationStyle(app);

    Database database("main.db");
    
    auto [ratingManager, teamManager, teamRepository, playerRepository] = 
        initializeServices(database);

    MainWindow mainWindow(*ratingManager, *teamManager, database);
    mainWindow.show();

    return app.exec();
}

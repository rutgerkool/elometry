#include "gui/MainWindow.h"
#include <QtWidgets/QApplication>
#include <QFile>
#include <QStyle>
#include <QStyleFactory>
#include "utils/database/Database.h"
#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/repositories/TeamRepository.h"

void applyStyle(QApplication& app) {
    app.setStyle(QStyleFactory::create("Fusion"));

    QFile styleFile(":/styles/styles.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString style = styleFile.readAll();
        app.setStyleSheet(style);
        styleFile.close();
    }

    QFont defaultFont("Segoe UI", 10);
    app.setFont(defaultFont);
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    applyStyle(app);

    Database database("test.db");
    RatingManager ratingManager(database);
    TeamRepository teamRepository(database);
    TeamManager teamManager(teamRepository, ratingManager);

    ratingManager.loadAndProcessRatings();

    MainWindow w(ratingManager, teamManager);
    w.show();

    return app.exec();
}
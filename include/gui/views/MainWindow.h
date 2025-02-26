#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/Database.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>

class PlayerListView;
class TeamManagerView;
class SettingsView;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(RatingManager& ratingManager, TeamManager& teamManager, Database& database, QWidget *parent = nullptr);
        ~MainWindow() override;

    private slots:
        void showPlayerList();
        void showTeamManager();
        void showSettings();

    private:
        RatingManager& ratingManager;
        TeamManager& teamManager;
        Database& database;
        
        PlayerListView* playerListView;
        TeamManagerView* teamManagerView;
        SettingsView* settingsView;

        QWidget* mainView;
        QStackedWidget* stackedWidget;

        void setupUi();
        void setupConnections();
        void showMainView();
};

#endif

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/Database.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>
#include <QThread>

class PlayerListView;
class TeamManagerView;
class SettingsView;
class LoadingView;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(RatingManager& ratingManager, TeamManager& teamManager, Database& database, QWidget *parent = nullptr);
        ~MainWindow() override;

    private slots:
        void showPlayerList();
        void showTeamManager();
        void showSettings();
        void showMainView();
        void initializeApp();
        void onDataLoadProgress(const QString& status, int progress);

    private:
        RatingManager& ratingManager;
        TeamManager& teamManager;
        Database& database;
        
        PlayerListView* playerListView;
        TeamManagerView* teamManagerView;
        SettingsView* settingsView;
        LoadingView* loadingView;

        QWidget* mainView;
        QStackedWidget* stackedWidget;
        
        QThread* loadingThread;
        bool appInitialized;

        void setupUi();
        void setupConnections();
};

#endif

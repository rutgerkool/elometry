#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include "utils/database/Database.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>
#include <QThread>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QShowEvent>

class PlayerListView;
class TeamManagerView;
class SettingsView;
class LoadingView;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(RatingManager& ratingManager, TeamManager& teamManager, Database& database, QWidget *parent = nullptr);
        ~MainWindow() override;

    protected:
        void showEvent(QShowEvent* event) override;

    private slots:
        void showPlayerList();
        void showTeamManager();
        void showSettings();
        void showMainView();
        void initializeApp();
        void onDataLoadProgress(const QString& status, int progress);
        void animateViewTransition(QWidget* newWidget);
        void centerWindow();

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

        QPropertyAnimation* fadeAnimation;
        QGraphicsOpacityEffect* opacityEffect;

        void setupUi();
        void setupConnections();
        void setupAnimations();
};

#endif

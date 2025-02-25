#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "services/RatingManager.h"
#include "services/TeamManager.h"
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStackedWidget>

class PlayerListView;
class TeamManagerView;

class MainWindow : public QMainWindow {
    Q_OBJECT

    public:
        explicit MainWindow(RatingManager& ratingManager, TeamManager& teamManager, QWidget *parent = nullptr);
        ~MainWindow() override;

    private slots:
        void showPlayerList();
        void showTeamManager();

    private:
        RatingManager& ratingManager;
        TeamManager& teamManager;
        
        PlayerListView* playerListView;
        TeamManagerView* teamManagerView;

        QWidget* mainView;
        QStackedWidget* stackedWidget;

        void setupUi();
        void setupConnections();
        void showMainView();
};

#endif

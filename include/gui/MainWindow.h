#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "services/RatingManager.h"
#include "services/TeamManager.h"

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
    void setupUi();

    RatingManager& ratingManager;
    TeamManager& teamManager;
    
    PlayerListView* playerListView;
    TeamManagerView* teamManagerView;
};

#endif

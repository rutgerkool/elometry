#ifndef TEAMMANAGERVIEW_H
#define TEAMMANAGERVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include "services/TeamManager.h"

class TeamListModel;

class TeamManagerView : public QWidget {
    Q_OBJECT

public:
    explicit TeamManagerView(TeamManager& teamManager, QWidget *parent = nullptr);

    private slots:
        void createNewTeam();
        void loadSelectedTeam();
        void autoFillTeam();
        void updateTeamInfo();

    private:
        void setupUi();
        void setupConnections();

        TeamManager& teamManager;
        TeamListModel* model;
        QListView* teamList;
        QListView* currentTeamPlayers;
        QPushButton* newTeamButton;
        QPushButton* loadTeamButton;
        QPushButton* autoFillButton;
        QLineEdit* teamNameInput;
        QSpinBox* budgetInput;

        Team* currentTeam;
};

#endif

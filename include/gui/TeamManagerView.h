#ifndef TEAMMANAGERVIEW_H
#define TEAMMANAGERVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QListView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include "services/TeamManager.h"

class TeamListModel;

class TeamManagerView : public QWidget {
    Q_OBJECT

public:
    explicit TeamManagerView(TeamManager& teamManager, QWidget *parent = nullptr);

signals:
        void backToMain(); 

private slots:
    void createNewTeam();
    void loadSelectedTeam();
    void autoFillTeam();
    void updateTeamInfo();
    void loadTeamById();
    void updateBudget(int newBudget);
    void removeSelectedPlayer();
    void navigateBack();

private:
    TeamManager& teamManager;
    Team* currentTeam;
    TeamListModel* model;

    QListView* teamList;
    QListView* currentTeamPlayers;
    QPushButton* newTeamButton;
    QPushButton* loadTeamByIdButton;
    QPushButton* autoFillButton;
    QPushButton* removePlayerButton;
    QPushButton* backButton;
    QLineEdit* teamNameInput;
    QLineEdit* loadTeamIdInput;
    QSpinBox* budgetInput;

    void setupUi();
    void setupConnections();
};

#endif

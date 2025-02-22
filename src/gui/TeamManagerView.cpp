#include "gui/TeamManagerView.h"
#include "gui/Models.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>

TeamManagerView::TeamManagerView(TeamManager& tm, QWidget *parent)
    : QWidget(parent)
    , teamManager(tm)
    , model(new TeamListModel(teamManager))
    , currentTeam(nullptr)
{
    setupUi();
    setupConnections();
}

void TeamManagerView::setupUi() {
    QHBoxLayout* mainLayout = new QHBoxLayout(this);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    
    QLabel* teamsLabel = new QLabel("Existing Teams:", this);
    teamList = new QListView(this);
    teamList->setModel(model);
    
    QHBoxLayout* newTeamLayout = new QHBoxLayout();
    teamNameInput = new QLineEdit(this);
    teamNameInput->setPlaceholderText("New Team Name");
    newTeamButton = new QPushButton("Create Team", this);
    newTeamLayout->addWidget(teamNameInput);
    newTeamLayout->addWidget(newTeamButton);

    loadTeamButton = new QPushButton("Load Selected Team", this);

    leftLayout->addWidget(teamsLabel);
    leftLayout->addWidget(teamList);
    leftLayout->addLayout(newTeamLayout);
    leftLayout->addWidget(loadTeamButton);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    
    QLabel* currentTeamLabel = new QLabel("Current Team:", this);
    currentTeamPlayers = new QListView(this);
    
    QHBoxLayout* budgetLayout = new QHBoxLayout();
    QLabel* budgetLabel = new QLabel("Budget (€):", this);
    budgetInput = new QSpinBox(this);
    budgetInput->setRange(0, 1000000000);  
    budgetInput->setValue(20000000);  
    budgetInput->setSingleStep(1000000);  
    budgetLayout->addWidget(budgetLabel);
    budgetLayout->addWidget(budgetInput);

    autoFillButton = new QPushButton("Auto-Fill Team", this);
    
    rightLayout->addWidget(currentTeamLabel);
    rightLayout->addWidget(currentTeamPlayers);
    rightLayout->addLayout(budgetLayout);
    rightLayout->addWidget(autoFillButton);

    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    currentTeamPlayers->setEnabled(false);
    autoFillButton->setEnabled(false);
    budgetInput->setEnabled(false);
}

void TeamManagerView::setupConnections() {
    connect(newTeamButton, &QPushButton::clicked, this, &TeamManagerView::createNewTeam);
    connect(loadTeamButton, &QPushButton::clicked, this, &TeamManagerView::loadSelectedTeam);
    connect(autoFillButton, &QPushButton::clicked, this, &TeamManagerView::autoFillTeam);
}

void TeamManagerView::createNewTeam() {
    QString name = teamNameInput->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a team name");
        return;
    }

    try {
        currentTeam = &teamManager.createTeam(name.toStdString());
        model->refresh();
        updateTeamInfo();
        
        currentTeamPlayers->setEnabled(true);
        autoFillButton->setEnabled(true);
        budgetInput->setEnabled(true);

        teamNameInput->clear();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to create team: %1").arg(e.what()));
    }
}

void TeamManagerView::loadSelectedTeam() {
    QModelIndex index = teamList->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Error", "Please select a team to load");
        return;
    }

    try {
        int teamId = model->data(index, Qt::UserRole).toInt();
        currentTeam = &teamManager.loadTeam(teamId);
        updateTeamInfo();
        
        currentTeamPlayers->setEnabled(true);
        autoFillButton->setEnabled(true);
        budgetInput->setEnabled(true);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to load team: %1").arg(e.what()));
    }
}

void TeamManagerView::autoFillTeam() {
    if (!currentTeam) {
        QMessageBox::warning(this, "Error", "No team loaded");
        return;
    }

    try {
        teamManager.autoFillTeam(*currentTeam, budgetInput->value());
        updateTeamInfo();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to auto-fill team: %1").arg(e.what()));
    }
}

void TeamManagerView::updateTeamInfo() {
    if (!currentTeam) return;
}

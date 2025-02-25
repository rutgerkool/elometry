#include "gui/TeamManagerView.h"
#include "gui/Models.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>

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
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    backButton = new QPushButton("Back to Menu", this);  
    mainLayout->addWidget(backButton, 0, Qt::AlignLeft);

    QHBoxLayout* mainContentLayout = new QHBoxLayout();

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

    QHBoxLayout* loadTeamLayout = new QHBoxLayout();
    loadTeamIdInput = new QLineEdit(this);
    loadTeamIdInput->setPlaceholderText("Enter Club ID");
    loadTeamByIdButton = new QPushButton("Load Club", this);
    loadTeamLayout->addWidget(loadTeamIdInput);
    loadTeamLayout->addWidget(loadTeamByIdButton);

    leftLayout->addWidget(teamsLabel);
    leftLayout->addWidget(teamList);
    leftLayout->addLayout(newTeamLayout);
    leftLayout->addLayout(loadTeamLayout);

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
    removePlayerButton = new QPushButton("Remove Player", this);

    rightLayout->addWidget(currentTeamLabel);
    rightLayout->addWidget(currentTeamPlayers);
    rightLayout->addLayout(budgetLayout);
    rightLayout->addWidget(autoFillButton);
    rightLayout->addWidget(removePlayerButton);

    mainContentLayout->addLayout(leftLayout);
    mainContentLayout->addLayout(rightLayout);
    mainLayout->addLayout(mainContentLayout);
    
    currentTeamPlayers->setEnabled(false);
    autoFillButton->setEnabled(false);
    budgetInput->setEnabled(false);
    removePlayerButton->setEnabled(false);
}

void TeamManagerView::setupConnections() {
    connect(newTeamButton, &QPushButton::clicked, this, &TeamManagerView::createNewTeam);
    connect(loadTeamByIdButton, &QPushButton::clicked, this, &TeamManagerView::loadTeamById);
    connect(autoFillButton, &QPushButton::clicked, this, &TeamManagerView::autoFillTeam);
    connect(budgetInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &TeamManagerView::updateBudget);
    connect(removePlayerButton, &QPushButton::clicked, this, &TeamManagerView::removeSelectedPlayer);
    connect(teamList->selectionModel(), &QItemSelectionModel::currentChanged, this, &TeamManagerView::loadSelectedTeam);
    connect(backButton, &QPushButton::clicked, this, &TeamManagerView::navigateBack);
}

void TeamManagerView::createNewTeam() {
    QString name = teamNameInput->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a team name");
        return;
    }

    try {
        Team& newTeam = teamManager.createTeam(name.toStdString());
        currentTeam = &newTeam;
        model->refresh();
        updateTeamInfo();
        
        currentTeamPlayers->setEnabled(true);
        autoFillButton->setEnabled(true);
        budgetInput->setEnabled(true);
        removePlayerButton->setEnabled(true);

        teamNameInput->clear();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to create team: %1").arg(e.what()));
        currentTeam = nullptr;
    }
}

void TeamManagerView::loadSelectedTeam() {
    QModelIndex index = teamList->currentIndex();
    if (!index.isValid()) return;

    try {
        int teamId = model->data(index, Qt::UserRole).toInt();
        Team& selectedTeam = teamManager.loadTeam(teamId);
        currentTeam = &selectedTeam;
        updateTeamInfo();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to load team: %1").arg(e.what()));
        currentTeam = nullptr;
    }
}

void TeamManagerView::loadTeamById() {
    bool ok;
    int clubId = loadTeamIdInput->text().toInt(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Error", "Invalid Club ID");
        return;
    }

    try {
        Team& selectedTeam = teamManager.loadTeamFromClub(clubId);
        currentTeam = &selectedTeam;
        model->refresh();
        updateTeamInfo();
        loadTeamIdInput->clear();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to load team: %1").arg(e.what()));
        currentTeam = nullptr;
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

void TeamManagerView::updateBudget(int newBudget) {
    if (currentTeam) {
        teamManager.setTeamBudget(currentTeam->teamId, newBudget);
    }
}

void TeamManagerView::removeSelectedPlayer() {
    QModelIndex index = currentTeamPlayers->currentIndex();
    if (!index.isValid() || !currentTeam) return;

    int playerId = index.data(Qt::UserRole).toInt();

    if (playerId == 0) {
        QMessageBox::warning(this, "Error", "Invalid player selected.");
        return;
    }

    teamManager.removePlayerFromTeam(currentTeam->teamId, playerId);
    updateTeamInfo();
}


void TeamManagerView::updateTeamInfo() {
    if (!currentTeam) {
        currentTeamPlayers->setModel(nullptr);
        budgetInput->setEnabled(false);
        return;
    }

    QStandardItemModel* playerModel = new QStandardItemModel(this);

    for (const auto& player : currentTeam->players) {
        QStandardItem* item = new QStandardItem(QString::fromStdString(player.name));
        item->setData(player.playerId, Qt::UserRole); 
        playerModel->appendRow(item);
    }

    currentTeamPlayers->setModel(playerModel);
    budgetInput->setEnabled(true);
    budgetInput->setValue(currentTeam->budget);
    autoFillButton->setEnabled(true);
}

void TeamManagerView::navigateBack() {
    emit backToMain();
}

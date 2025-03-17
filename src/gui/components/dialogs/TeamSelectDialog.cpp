#include "gui/components/dialogs/TeamSelectDialog.h"
#include "gui/models/TeamSelectModel.h"
#include <QtWidgets/QMessageBox>
#include <algorithm>

TeamSelectDialog::TeamSelectDialog(TeamManager& tm, int playerId, QWidget *parent)
    : QDialog(parent)
    , teamManager(tm)
    , playerId(playerId)
    , teamsModel(nullptr)
{
    setupUi();
    setupConnections();
    updateTeamList();
    
    setWindowTitle("Add to Team");
    resize(400, 500);
}

void TeamSelectDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    setupTitleAndLabels();
    setupSearchControls();
    setupListView();
    setupButtons();
    
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(selectionInfoLabel);
    mainLayout->addLayout(createSearchLayout());
    mainLayout->addWidget(instructionLabel);
    mainLayout->addWidget(teamsList, 1);
    mainLayout->addWidget(noTeamsLabel);
    mainLayout->addWidget(buttonBox);
}

QHBoxLayout* TeamSelectDialog::createSearchLayout() {
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(8);
    searchLayout->addWidget(searchInput, 1);
    searchLayout->addWidget(clearButton);
    return searchLayout;
}

void TeamSelectDialog::setupTitleAndLabels() {
    titleLabel = new QLabel("Add Player to Teams", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px; color: #e0e0e0;");
    
    selectionInfoLabel = new QLabel("Selected: 0 teams", this);
    selectionInfoLabel->setStyleSheet("font-weight: bold; color: #e0e0e0;");
    
    instructionLabel = new QLabel(
        "Select the teams you want to add this player to", this);
    instructionLabel->setStyleSheet("color: #b0b0b0; font-style: italic;");
    
    noTeamsLabel = new QLabel("No teams available. Please create a team first.", this);
    noTeamsLabel->setStyleSheet("color: #b0b0b0; font-style: italic; padding: 20px; text-align: center;");
    noTeamsLabel->setAlignment(Qt::AlignCenter);
    noTeamsLabel->hide();
}

void TeamSelectDialog::setupSearchControls() {
    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Search teams");
    searchInput->setMinimumHeight(28);
    
    clearButton = new QPushButton("Clear", this);
    clearButton->setMinimumHeight(28);
    clearButton->setCursor(Qt::PointingHandCursor);
}

void TeamSelectDialog::setupListView() {
    teamsList = new QListView(this);
    teamsList->setSelectionMode(QAbstractItemView::SingleSelection);
    teamsList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    teamsList->setAlternatingRowColors(true);
}

void TeamSelectDialog::setupButtons() {
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Add to Selected Teams");
    buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    buttonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
}

void TeamSelectDialog::setupConnections() {
    connect(searchInput, &QLineEdit::textChanged, this, &TeamSelectDialog::searchTeams);
    connect(clearButton, &QPushButton::clicked, this, &TeamSelectDialog::clearFilter);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    connect(teamsList, &QListView::clicked, this, &TeamSelectDialog::toggleSelection);
}

void TeamSelectDialog::updateTeamList() {
    std::vector<Team> allTeams = teamManager.getAllTeams();
    
    if (allTeams.empty()) {
        handleEmptyTeamList();
        return;
    }
    
    handleNonEmptyTeamList(allTeams);
    updateInitialTeamIds();
    updateSelectionInfo();
}

void TeamSelectDialog::handleEmptyTeamList() {
    teamsList->hide();
    noTeamsLabel->show();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

void TeamSelectDialog::handleNonEmptyTeamList(const std::vector<Team>& allTeams) {
    noTeamsLabel->hide();
    teamsList->show();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    
    QAbstractItemModel* oldModel = teamsList->model();
    teamsModel = new TeamSelectModel(allTeams, playerId, this);
    teamsList->setModel(teamsModel);
    
    if (oldModel && oldModel != teamsModel) {
        delete oldModel;
    }
    
    connect(teamsModel, &QAbstractItemModel::dataChanged, 
            this, &TeamSelectDialog::updateSelectionInfo);
}

void TeamSelectDialog::updateInitialTeamIds() {
    initialTeamIds.clear();
    if (teamsModel) {
        for (int id : teamsModel->getInitialSelectedTeamIds()) {
            initialTeamIds.insert(id);
        }
    }
}

void TeamSelectDialog::searchTeams(const QString& text) {
    if (teamsModel) {
        teamsModel->setFilter(text);
    }
}

void TeamSelectDialog::clearFilter() {
    searchInput->clear();
    if (teamsModel) {
        teamsModel->setFilter("");
    }
}

void TeamSelectDialog::toggleSelection(const QModelIndex &index) {
    if (!index.isValid() || !teamsModel) {
        return;
    }
    
    QVariant checkState = index.data(Qt::CheckStateRole);
    teamsModel->setData(index, 
                       checkState.toInt() == Qt::Checked ? Qt::Unchecked : Qt::Checked, 
                       Qt::CheckStateRole);
    
    teamsList->setCurrentIndex(index);
}

void TeamSelectDialog::updateSelectionInfo() {
    if (!teamsModel) {
        return;
    }
    
    int count = teamsModel->getSelectedCount();
    int newCount = 0;
    int removedCount = 0;
    
    for (int id : teamsModel->getSelectedTeamIds()) {
        if (!initialTeamIds.contains(id)) {
            newCount++;
        }
    }
    
    for (int id : initialTeamIds) {
        if (!teamsModel->isTeamSelected(id)) {
            removedCount++;
        }
    }
    
    QString selectionText = buildSelectionInfoText(count, newCount, removedCount);
    selectionInfoLabel->setText(selectionText);
}

QString TeamSelectDialog::buildSelectionInfoText(int count, int newCount, int removedCount) {
    QString selectionText = QString("Selected: %1 team%2")
        .arg(count)
        .arg(count == 1 ? "" : "s");
    
    if (newCount <= 0 && removedCount <= 0) {
        return selectionText;
    }
    
    selectionText += " (";
    
    if (newCount > 0) {
        selectionText += QString("%1 to add").arg(newCount);
        if (removedCount > 0) {
            selectionText += ", ";
        }
    }
    
    if (removedCount > 0) {
        selectionText += QString("%1 to remove").arg(removedCount);
    }
    
    selectionText += ")";
    return selectionText;
}

std::vector<int> TeamSelectDialog::getSelectedTeamIds() const {
    if (!teamsModel) {
        return std::vector<int>();
    }
    return teamsModel->getSelectedTeamIds();
}

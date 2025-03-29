#include "gui/components/dialogs/TeamSelectDialog.h"
#include "gui/models/TeamSelectModel.h"
#include "services/TeamManager.h"

#include <QtWidgets/QListView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>

#include <algorithm>
#include <ranges>

TeamSelectDialog::TeamSelectDialog(TeamManager& teamManager, int playerId, QWidget* parent)
    : QDialog(parent)
    , m_teamManager(teamManager)
    , m_playerId(playerId)
{
    setupUi();
    setupConnections();
    loadTeams();
    
    setWindowTitle(tr("Add to Team"));
    resize(400, 500);
}

void TeamSelectDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    createHeaderSection();
    createSearchSection();
    createTeamsListSection();
    createButtonSection();
    
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_selectionInfoLabel);
    
    auto* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(8);
    searchLayout->addWidget(m_searchInput, 1);
    searchLayout->addWidget(m_clearButton);
    mainLayout->addLayout(searchLayout);
    
    mainLayout->addWidget(m_instructionLabel);
    mainLayout->addWidget(m_teamsList, 1);
    mainLayout->addWidget(m_noTeamsLabel);
    mainLayout->addWidget(m_buttonBox);
}

void TeamSelectDialog::createHeaderSection() {
    m_titleLabel = new QLabel(tr("Add Player to Teams"), this);
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px; color: #e0e0e0;");
    
    m_selectionInfoLabel = new QLabel(tr("Selected: 0 teams"), this);
    m_selectionInfoLabel->setStyleSheet("font-weight: bold; color: #e0e0e0;");
    
    m_instructionLabel = new QLabel(
        tr("Select the teams you want to add this player to"), this);
    m_instructionLabel->setStyleSheet("color: #b0b0b0; font-style: italic;");
}

void TeamSelectDialog::createSearchSection() {
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(tr("Search teams"));
    m_searchInput->setMinimumHeight(28);
    
    m_clearButton = new QPushButton(tr("Clear"), this);
    m_clearButton->setMinimumHeight(28);
    m_clearButton->setCursor(Qt::PointingHandCursor);
}

void TeamSelectDialog::createTeamsListSection() {
    m_teamsList = new QListView(this);
    m_teamsList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_teamsList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_teamsList->setAlternatingRowColors(true);
    
    m_noTeamsLabel = new QLabel(tr("No teams available. Please create a team first."), this);
    m_noTeamsLabel->setStyleSheet("color: #b0b0b0; font-style: italic; padding: 20px; text-align: center;");
    m_noTeamsLabel->setAlignment(Qt::AlignCenter);
    m_noTeamsLabel->hide();
}

void TeamSelectDialog::createButtonSection() {
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Add to Selected Teams"));
    m_buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    m_buttonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
}

void TeamSelectDialog::setupConnections() {
    connect(m_searchInput, &QLineEdit::textChanged, this, &TeamSelectDialog::filterTeams);
    connect(m_clearButton, &QPushButton::clicked, this, &TeamSelectDialog::clearFilter);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_teamsList, &QListView::clicked, this, &TeamSelectDialog::toggleSelection);
}

void TeamSelectDialog::loadTeams() {
    auto allTeams = m_teamManager.getAllTeams();
    
    if (allTeams.empty()) {
        m_teamsList->hide();
        m_noTeamsLabel->show();
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }
    
    m_noTeamsLabel->hide();
    m_teamsList->show();
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    
    delete m_teamsList->model();
    m_teamsModel = new TeamSelectModel(allTeams, m_playerId, this);
    m_teamsList->setModel(m_teamsModel);
    
    connect(m_teamsModel, &QAbstractItemModel::dataChanged, 
            this, &TeamSelectDialog::updateSelectionInfo);
    
    m_initialTeamIds.clear();
    for (int id : m_teamsModel->getInitialSelectedTeamIds()) {
        m_initialTeamIds.insert(id);
    }
    
    updateSelectionInfo();
}

void TeamSelectDialog::filterTeams(const QString& filter) {
    if (m_teamsModel) {
        m_teamsModel->setFilter(filter);
    }
}

void TeamSelectDialog::clearFilter() {
    m_searchInput->clear();
    if (m_teamsModel) {
        m_teamsModel->setFilter("");
    }
}

void TeamSelectDialog::toggleSelection(const QModelIndex& index) {
    if (!index.isValid() || !m_teamsModel) {
        return;
    }
    
    QVariant checkState = index.data(Qt::CheckStateRole);
    
    const bool success = m_teamsModel->setData(
        index, 
        checkState.toInt() == Qt::Checked ? Qt::Unchecked : Qt::Checked,
        Qt::CheckStateRole
    );
    
    if (!success) {
        return;
    }
    
    m_teamsList->setCurrentIndex(index);
}

void TeamSelectDialog::updateSelectionInfo() {
    if (!m_teamsModel) {
        return;
    }
    
    int selectedCount = m_teamsModel->getSelectedCount();
    auto [newCount, removedCount] = calculateTeamChanges();
    
    QString infoText = formatSelectionText(selectedCount, newCount, removedCount);
    m_selectionInfoLabel->setText(infoText);
}

std::pair<int, int> TeamSelectDialog::calculateTeamChanges() const {
    int newCount = 0;
    int removedCount = 0;
    
    for (int id : m_teamsModel->getSelectedTeamIds()) {
        if (!m_initialTeamIds.contains(id)) {
            newCount++;
        }
    }
    
    for (int id : m_initialTeamIds) {
        if (!m_teamsModel->isTeamSelected(id)) {
            removedCount++;
        }
    }
    
    return {newCount, removedCount};
}

QString TeamSelectDialog::formatSelectionText(int selectedCount, int newCount, int removedCount) const {
    QString selectionText = tr("Selected: %1 team%2")
        .arg(selectedCount)
        .arg(selectedCount == 1 ? "" : "s");
    
    if (newCount <= 0 && removedCount <= 0) {
        return selectionText;
    }
    
    selectionText += " (";
    
    if (newCount > 0) {
        selectionText += tr("%1 to add").arg(newCount);
        if (removedCount > 0) {
            selectionText += ", ";
        }
    }
    
    if (removedCount > 0) {
        selectionText += tr("%1 to remove").arg(removedCount);
    }
    
    selectionText += ")";
    return selectionText;
}

std::vector<int> TeamSelectDialog::getSelectedTeamIds() const {
    if (!m_teamsModel) {
        return {};
    }
    return m_teamsModel->getSelectedTeamIds();
}

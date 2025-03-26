#include "gui/components/dialogs/PlayerSelectDialog.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QApplication>
#include <QTimer>

#include <algorithm>
#include <ranges>

PlayerSelectDialog::PlayerSelectDialog(TeamManager& teamManager, Team* currentTeam, QWidget* parent)
    : QDialog(parent)
    , m_teamManager(teamManager)
    , m_currentTeam(currentTeam)
    , m_playersModel(nullptr)
{
    setupUi();
    initializePositionFilter();
    setupConnections();
    updatePlayerList();
    
    if (m_currentTeam) {
        selectCurrentTeamPlayers();
    }
    
    setWindowTitle(tr("Select Players"));
    resize(900, 650);
}

void PlayerSelectDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    setupTitleAndLabels();
    setupSearchControls();
    setupTableView();
    setupButtons();
    
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_selectionInfoLabel);
    mainLayout->addLayout(createSearchLayout());
    mainLayout->addWidget(m_instructionLabel);
    mainLayout->addWidget(m_playersTable, 1);
    mainLayout->addWidget(m_loadingIndicator);
    mainLayout->addWidget(m_buttonBox);
}

void PlayerSelectDialog::setupTitleAndLabels() {
    m_titleLabel = new QLabel(tr("Player Search"), this);
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px; color: #e0e0e0;");
    
    m_selectionInfoLabel = new QLabel(tr("Selected: 0 players"), this);
    m_selectionInfoLabel->setStyleSheet("font-weight: bold; color: #e0e0e0;");
    
    m_instructionLabel = new QLabel(
        tr("Double-click on a row or checkbox to select players to add to your team"), this);
    m_instructionLabel->setStyleSheet("color: #b0b0b0; font-style: italic;");
    
    m_loadingIndicator = new QLabel(tr("Loading more players..."), this);
    m_loadingIndicator->setStyleSheet("color: #b0b0b0; font-style: italic; padding: 5px;");
    m_loadingIndicator->setAlignment(Qt::AlignCenter);
    m_loadingIndicator->hide();
}

QHBoxLayout* PlayerSelectDialog::createSearchLayout() {
    auto* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(8);
    
    searchLayout->addWidget(m_searchInput, 3);
    searchLayout->addWidget(m_positionFilter, 2);
    searchLayout->addWidget(m_clearButton);
    
    return searchLayout;
}

void PlayerSelectDialog::setupSearchControls() {
    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(tr("Enter player name"));
    m_searchInput->setMinimumHeight(28);
    
    m_positionFilter = new QComboBox(this);
    m_positionFilter->setMinimumWidth(140);
    m_positionFilter->setMinimumHeight(28);
    
    m_clearButton = new QPushButton(tr("Clear Filters"), this);
    m_clearButton->setMinimumHeight(28);
    m_clearButton->setCursor(Qt::PointingHandCursor);
}

void PlayerSelectDialog::setupTableView() {
    m_playersTable = new QTableView(this);
    m_playersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_playersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_playersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_playersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_playersTable->verticalHeader()->setVisible(false);
    m_playersTable->setSortingEnabled(true);
    m_playersTable->setShowGrid(false);
    m_playersTable->setAlternatingRowColors(true);
    m_playersTable->horizontalHeader()->setStretchLastSection(true);
}

void PlayerSelectDialog::setupButtons() {
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Add Selected Players"));
    m_buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    m_buttonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
}

void PlayerSelectDialog::setupConnections() {
    connect(m_searchInput, &QLineEdit::textChanged, this, &PlayerSelectDialog::searchPlayers);
    connect(m_positionFilter, &QComboBox::currentIndexChanged, this, &PlayerSelectDialog::positionFilterChanged);
    connect(m_clearButton, &QPushButton::clicked, this, &PlayerSelectDialog::clearFilters);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    setupScrollConnections();
    setupTableConnections();
}

void PlayerSelectDialog::setupScrollConnections() {
    connect(m_playersTable->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, &PlayerSelectDialog::checkScrollPosition);
}

void PlayerSelectDialog::setupTableConnections() {
    connect(m_playersTable, &QTableView::doubleClicked, this, &PlayerSelectDialog::toggleSelection);
    
    connect(m_playersTable, &QTableView::clicked, this, [this](const QModelIndex &index) {
        if (index.column() == 0) {
            toggleSelection();
        }
    });
    
    connect(m_playersTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, &PlayerSelectDialog::sortPlayerList);
}

void PlayerSelectDialog::initializePositionFilter() {
    m_positionFilter->addItem(tr("All Positions"));
    
    std::vector<std::string> positions = m_teamManager.getAvailableSubPositions();
    for (const auto& position : positions) {
        m_positionFilter->addItem(QString::fromStdString(position));
    }
}

void PlayerSelectDialog::createPlayerSelectModel() {
    std::vector<std::pair<int, Player>> allPlayers;
    std::vector<Player> players = m_teamManager.getRatingManager().getAllPlayers();
    
    allPlayers.reserve(players.size());
    for (const auto& player : players) {
        allPlayers.emplace_back(player.playerId, player);
    }
    
    m_playersModel = std::make_unique<PlayerSelectModel>(allPlayers, this);
    m_playersTable->setModel(m_playersModel.get());
    
    connect(m_playersModel.get(), &QAbstractItemModel::dataChanged, 
            this, &PlayerSelectDialog::updateSelectionInfo);
}

void PlayerSelectDialog::configureTableColumns() {
    m_playersTable->setColumnWidth(0, 40);
    m_playersTable->setColumnWidth(1, 60);
    m_playersTable->setColumnWidth(2, 250);
    m_playersTable->setColumnWidth(3, 80);
    m_playersTable->setColumnWidth(4, 100);
}

void PlayerSelectDialog::selectCurrentTeamPlayers() {
    if (!m_currentTeam || !m_playersModel) {
        return;
    }
    
    for (const auto& player : m_currentTeam->players) {
        m_playersModel->selectPlayer(player.playerId);
    }
    updateSelectionInfo();
}

void PlayerSelectDialog::updatePlayerList() {
    m_currentOffset = 0;
    m_isLoading = false;
    
    createPlayerSelectModel();
    
    if (m_currentTeam) {
        selectCurrentTeamPlayers();
    }
    
    configureTableColumns();
    
    m_playersModel->setPagination(0, m_pageSize);
    updateSelectionInfo();
    
    m_playersTable->horizontalHeader()->setSortIndicator(0, Qt::DescendingOrder);
    m_playersTable->verticalScrollBar()->setValue(0);
}

void PlayerSelectDialog::searchPlayers() {
    if (!m_playersModel) {
        return;
    }
    
    m_loadingIndicator->hide();
    m_isLoading = false;
    
    const QString searchText = m_searchInput->text().trimmed();
    m_playersModel->setFilter(searchText);
    m_playersModel->setPagination(0, m_pageSize);
    m_currentOffset = 0;
    
    m_playersTable->horizontalHeader()->setSortIndicator(0, Qt::DescendingOrder);
}

void PlayerSelectDialog::positionFilterChanged() {
    if (!m_playersModel) {
        return;
    }
    
    m_loadingIndicator->hide();
    m_isLoading = false;
    
    QString posFilter = m_positionFilter->currentText();
    if (posFilter == tr("All Positions")) {
        posFilter.clear();
    }
    
    m_playersModel->setPositionFilter(posFilter);
    m_playersModel->setPagination(0, m_pageSize);
    m_currentOffset = 0;
    
    m_playersTable->horizontalHeader()->setSortIndicator(0, Qt::DescendingOrder);
}

void PlayerSelectDialog::clearFilters() {
    m_searchInput->clear();
    m_positionFilter->setCurrentIndex(0);
    
    m_loadingIndicator->hide();
    m_isLoading = false;
    
    if (m_playersModel) {
        m_playersModel->setFilter("");
        m_playersModel->setPositionFilter("");
        m_playersModel->setPagination(0, m_pageSize);
        m_currentOffset = 0;
        
        m_playersTable->horizontalHeader()->setSortIndicator(0, Qt::DescendingOrder);
    }
}

void PlayerSelectDialog::toggleSelection() {
    QModelIndex index = m_playersTable->currentIndex();
    if (!index.isValid() || !m_playersModel) {
        return;
    }
    
    QModelIndex checkboxIndex = m_playersModel->index(index.row(), 0);
    Qt::CheckState currentState = static_cast<Qt::CheckState>(
        m_playersModel->data(checkboxIndex, Qt::CheckStateRole).toInt());
    
    m_playersModel->setData(checkboxIndex, 
                         currentState == Qt::Checked ? Qt::Unchecked : Qt::Checked, 
                         Qt::CheckStateRole);
    
    m_playersTable->setCurrentIndex(index);
}

void PlayerSelectDialog::updateSelectionInfo() {
    if (!m_playersModel) {
        return;
    }
    
    int count = m_playersModel->getSelectedCount();
    m_selectionInfoLabel->setText(tr("Selected: %1 player%2")
                                .arg(count)
                                .arg(count == 1 ? "" : "s"));
}

void PlayerSelectDialog::sortPlayerList(int column, Qt::SortOrder order) {
    if (m_playersModel) {
        m_playersModel->sort(column, order);
    }
}

void PlayerSelectDialog::checkScrollPosition() {
    if (!m_playersModel) {
        return;
    }
    
    QScrollBar* vScrollBar = m_playersTable->verticalScrollBar();
    if (!vScrollBar) {
        return;
    }
    
    if (vScrollBar->value() >= vScrollBar->maximum() - 5) {
        loadMorePlayersIfNeeded();
    }
}

void PlayerSelectDialog::loadMorePlayersIfNeeded() {
    if (!m_playersModel || m_isLoading) {
        return;
    }
    
    int totalFiltered = m_playersModel->filteredPlayerCount();
    bool hasMore = (m_currentOffset + m_pageSize) < totalFiltered;
    
    if (hasMore) {
        m_isLoading = true;
        m_loadingIndicator->show();
        
        QModelIndex currentIndex = m_playersTable->currentIndex();
        int scrollValue = m_playersTable->verticalScrollBar()->value();
        
        QTimer::singleShot(200, this, [this, scrollValue, currentIndex]() {
            if ((m_currentOffset + m_pageSize) < m_playersModel->filteredPlayerCount()) {
                m_currentOffset += m_pageSize;
                
                m_playersModel->setPagination(0, m_currentOffset + m_pageSize);
                
                if (currentIndex.isValid()) {
                    m_playersTable->setCurrentIndex(currentIndex);
                }
                
                QTimer::singleShot(0, this, [this, scrollValue]() {
                    m_playersTable->verticalScrollBar()->setValue(scrollValue);
                });
            }
            
            m_loadingIndicator->hide();
            m_isLoading = false;
        });
    }
}

std::vector<Player> PlayerSelectDialog::getSelectedPlayers() const {
    if (!m_playersModel) {
        return {};
    }
    return m_playersModel->getSelectedPlayers();
}

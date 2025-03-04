#include "gui/components/PlayerSelectDialog.h"
#include <QtWidgets/QMessageBox>
#include <QtGui/QStandardItem>
#include <QtGui/QIcon>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QTimer>
#include <algorithm>

PlayerSelectDialog::PlayerSelectDialog(TeamManager& tm, Team* currentTeam, QWidget *parent)
    : QDialog(parent)
    , teamManager(tm)
    , currentTeam(currentTeam)
    , playersModel(nullptr)
    , isLoading(false)
{
    setupUi();
    initializePositionFilter();
    setupConnections();
    updatePlayerList();
    
    if (currentTeam) {
        for (const auto& player : currentTeam->players) {
            playersModel->selectPlayer(player.playerId);
        }
        updateSelectionInfo();
    }
    
    setWindowTitle("Select Players");
    resize(900, 650);
}

void PlayerSelectDialog::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    titleLabel = new QLabel("Player Search", this);
    titleLabel->setStyleSheet("font-size: 16px; font-weight: bold; margin-bottom: 10px; color: #e0e0e0;");
    
    selectionInfoLabel = new QLabel("Selected: 0 players", this);
    selectionInfoLabel->setStyleSheet("font-weight: bold; color: #e0e0e0;");
    
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(8);
    
    searchInput = new QLineEdit(this);
    searchInput->setPlaceholderText("Enter player name");
    searchInput->setMinimumHeight(28);
    
    positionFilter = new QComboBox(this);
    positionFilter->setMinimumWidth(140);
    positionFilter->setMinimumHeight(28);
    
    clearButton = new QPushButton("Clear Filters", this);
    clearButton->setMinimumHeight(28);
    clearButton->setCursor(Qt::PointingHandCursor);
    
    searchLayout->addWidget(searchInput, 3);
    searchLayout->addWidget(positionFilter, 2);
    searchLayout->addWidget(clearButton);
    
    playersTable = new QTableView(this);
    playersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    playersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    playersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    playersTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    playersTable->verticalHeader()->setVisible(false);
    playersTable->setSortingEnabled(true);
    playersTable->setShowGrid(false);
    playersTable->setAlternatingRowColors(true);
    playersTable->horizontalHeader()->setStretchLastSection(true);
    
    playersTable->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #2a2a2a;"
        "    color: #e0e0e0;"
        "    padding: 6px;"
        "    border: none;"
        "    border-bottom: 1px solid #3a3a3a;"
        "    font-weight: bold;"
        "}");
    
    instructionLabel = new QLabel(
        "Double-click on a row or checkbox to select players to add to your team", this);
    instructionLabel->setStyleSheet("color: #b0b0b0; font-style: italic;");
    
    loadingIndicator = new QLabel("Loading more players...", this);
    loadingIndicator->setStyleSheet("color: #b0b0b0; font-style: italic; padding: 5px;");
    loadingIndicator->setAlignment(Qt::AlignCenter);
    loadingIndicator->hide();
    
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Add Selected Players");
    buttonBox->button(QDialogButtonBox::Ok)->setCursor(Qt::PointingHandCursor);
    buttonBox->button(QDialogButtonBox::Cancel)->setCursor(Qt::PointingHandCursor);
    
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(selectionInfoLabel);
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(instructionLabel);
    mainLayout->addWidget(playersTable, 1);
    mainLayout->addWidget(loadingIndicator);
    mainLayout->addWidget(buttonBox);
}

void PlayerSelectDialog::setupConnections() {
    connect(searchInput, &QLineEdit::textChanged, this, &PlayerSelectDialog::searchPlayers);
    
    connect(positionFilter, &QComboBox::currentIndexChanged, this, &PlayerSelectDialog::positionFilterChanged);
    connect(clearButton, &QPushButton::clicked, this, &PlayerSelectDialog::clearFilters);
    
    connect(playersTable->verticalScrollBar(), &QScrollBar::valueChanged, 
            this, &PlayerSelectDialog::checkScrollPosition);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    connect(playersTable, &QTableView::doubleClicked, this, &PlayerSelectDialog::toggleSelection);
    
    connect(playersTable, &QTableView::clicked, [this](const QModelIndex &index) {
        if (index.column() == 0) {
            toggleSelection();
        }
    });
    
    connect(playersTable->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, &PlayerSelectDialog::sortPlayerList);
}

void PlayerSelectDialog::initializePositionFilter() {
    positionFilter->addItem("All Positions");
    
    std::vector<std::string> positions = teamManager.getAvailableSubPositions();
    for (const auto& position : positions) {
        positionFilter->addItem(QString::fromStdString(position));
    }
}

void PlayerSelectDialog::updatePlayerList() {
    currentOffset = 0;
    
    std::vector<std::pair<int, Player>> allPlayers;
    std::vector<Player> players = teamManager.getRatingManager().getAllPlayers();
    
    for (const auto& player : players) {
        allPlayers.emplace_back(player.playerId, player);
    }
    
    QAbstractItemModel* oldModel = playersTable->model();
    playersModel = new PlayerSelectModel(allPlayers, this);
    playersTable->setModel(playersModel);
    
    if (currentTeam) {
        for (const auto& player : currentTeam->players) {
            playersModel->selectPlayer(player.playerId);
        }
    }
    
    playersTable->setColumnWidth(0, 40);
    playersTable->setColumnWidth(1, 60);
    playersTable->setColumnWidth(2, 250);
    playersTable->setColumnWidth(3, 80);
    playersTable->setColumnWidth(4, 100);
    playersTable->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    
    playersModel->setPagination(0, pageSize);
    
    if (oldModel && oldModel != playersModel) {
        delete oldModel;
    }
    
    connect(playersModel, &QAbstractItemModel::dataChanged, this, &PlayerSelectDialog::updateSelectionInfo);
    
    updateSelectionInfo();
    
    playersTable->horizontalHeader()->setSortIndicator(0, Qt::DescendingOrder);
    
    playersTable->verticalScrollBar()->setValue(0);
}

void PlayerSelectDialog::searchPlayers() {
    if (!playersModel) return;
    
    loadingIndicator->hide();
    isLoading = false;
    
    QString searchText = searchInput->text().trimmed();
    playersModel->setFilter(searchText);
    playersModel->setPagination(0, pageSize);
    currentOffset = 0;
    
    playersTable->horizontalHeader()->setSortIndicator(0, Qt::DescendingOrder);
}

void PlayerSelectDialog::positionFilterChanged() {
    if (!playersModel) return;
    
    loadingIndicator->hide();
    isLoading = false;
    
    QString posFilter = positionFilter->currentText();
    if (posFilter == "All Positions") {
        posFilter = "";
    }
    
    playersModel->setPositionFilter(posFilter);
    playersModel->setPagination(0, pageSize);
    currentOffset = 0;
    
    playersTable->horizontalHeader()->setSortIndicator(0, Qt::DescendingOrder);
}

void PlayerSelectDialog::clearFilters() {
    searchInput->clear();
    positionFilter->setCurrentIndex(0);
    
    loadingIndicator->hide();
    isLoading = false;
    
    if (playersModel) {
        playersModel->setFilter("");
        playersModel->setPositionFilter("");
        playersModel->setPagination(0, pageSize);
        currentOffset = 0;
        
        playersTable->horizontalHeader()->setSortIndicator(0, Qt::DescendingOrder);
    }
}

void PlayerSelectDialog::toggleSelection() {
    QModelIndex index = playersTable->currentIndex();
    if (!index.isValid() || !playersModel) return;
    
    QModelIndex checkboxIndex = playersModel->index(index.row(), 0);
    Qt::CheckState currentState = static_cast<Qt::CheckState>(
        playersModel->data(checkboxIndex, Qt::CheckStateRole).toInt());
    
    playersModel->setData(checkboxIndex, 
                         currentState == Qt::Checked ? Qt::Unchecked : Qt::Checked, 
                         Qt::CheckStateRole);
    
    playersTable->setCurrentIndex(index);
}

void PlayerSelectDialog::updateSelectionInfo() {
    if (!playersModel) return;
    
    int count = playersModel->getSelectedCount();
    selectionInfoLabel->setText(QString("Selected: %1 player%2")
                                .arg(count)
                                .arg(count == 1 ? "" : "s"));
}

void PlayerSelectDialog::sortPlayerList(int column, Qt::SortOrder order) {
    if (playersModel) {
        playersModel->sort(column, order);
    }
}

void PlayerSelectDialog::checkScrollPosition() {
    if (!playersModel) return;
    
    QScrollBar* vScrollBar = playersTable->verticalScrollBar();
    if (!vScrollBar) return;
    
    if (vScrollBar->value() >= vScrollBar->maximum() - 5) {
        loadMorePlayersIfNeeded();
    }
}

void PlayerSelectDialog::loadMorePlayersIfNeeded() {
    if (!playersModel || isLoading) return;
    
    int totalFiltered = playersModel->filteredPlayerCount();
    bool hasMore = (currentOffset + pageSize) < totalFiltered;
    
    if (hasMore) {
        isLoading = true;
        loadingIndicator->show();
        
        QModelIndex currentIndex = playersTable->currentIndex();
        int scrollValue = playersTable->verticalScrollBar()->value();
        
        QTimer::singleShot(200, this, [this, scrollValue, currentIndex]() {
            if ((currentOffset + pageSize) < playersModel->filteredPlayerCount()) {
                currentOffset += pageSize;
                
                playersModel->setPagination(0, currentOffset + pageSize);
                
                if (currentIndex.isValid()) {
                    playersTable->setCurrentIndex(currentIndex);
                }
                
                QTimer::singleShot(0, this, [this, scrollValue]() {
                    playersTable->verticalScrollBar()->setValue(scrollValue);
                });
            }
            
            loadingIndicator->hide();
            isLoading = false;
        });
    }
}

std::vector<Player> PlayerSelectDialog::getSelectedPlayers() const {
    if (!playersModel) return std::vector<Player>();
    return playersModel->getSelectedPlayers();
}

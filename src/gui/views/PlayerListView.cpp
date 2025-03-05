#include "gui/views/PlayerListView.h"
#include "gui/models/PlayerListModel.h"
#include "gui/components/PlayerHistoryDialog.h"
#include "gui/components/PlayerComparisonDialog.h"
#include "gui/components/TeamSelectDialog.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QScrollArea>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <set>

PlayerListView::PlayerListView(RatingManager& rm, TeamManager& tm, QWidget *parent)
    : QWidget(parent)
    , ratingManager(rm)
    , teamManager(tm)
    , model(new PlayerListModel(ratingManager.getSortedRatedPlayers()))
    , networkManager(new QNetworkAccessManager(this))
    , comparisonPlayerId(-1)
{
    setupUi();
    setupAnimations();
    setupConnections();
    updatePagination();
    animateTable();
}

void PlayerListView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    backButton = new QPushButton("Back to Menu", this);
    mainLayout->addWidget(backButton, 0, Qt::AlignLeft);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);
    
    QVBoxLayout* leftSideLayout = new QVBoxLayout();
    leftSideLayout->setSpacing(15);
    
    setupInputSection();
    leftSideLayout->addLayout(createInputLayout());
    
    QFrame* tableFrame = setupTableSection();
    leftSideLayout->addWidget(tableFrame, 1);
    
    contentLayout->addLayout(leftSideLayout, 3);
    
    QScrollArea* playerDetailsScrollArea = setupPlayerDetailsSection();
    contentLayout->addWidget(playerDetailsScrollArea, 1);
    
    mainLayout->addLayout(contentLayout, 1);
}

void PlayerListView::setupInputSection() {
    searchBox = new QLineEdit(this);
    searchBox->setObjectName("searchBox");
    searchBox->setFixedWidth(240);
    searchBox->setPlaceholderText("Search players");

    positionFilter = new QComboBox(this);
    positionFilter->setObjectName("positionFilter");
    positionFilter->addItem("All Positions");
    positionFilter->addItems({"Goalkeeper", "Defender", "Midfield", "Attack"});
    positionFilter->setFixedWidth(240);
}

QHBoxLayout* PlayerListView::createInputLayout() {
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel("Search:", this);
    searchLabel->setObjectName("searchLabel");
    searchLabel->setProperty("section", "search");
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchBox);
    searchLayout->addStretch();

    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* positionLabel = new QLabel("Position:", this);
    positionLabel->setObjectName("positionLabel");
    positionLabel->setProperty("section", "position");
    filterLayout->addWidget(positionLabel);
    filterLayout->addWidget(positionFilter);
    filterLayout->addStretch();

    inputLayout->addLayout(searchLayout);
    inputLayout->addSpacing(20);
    inputLayout->addLayout(filterLayout);
    inputLayout->addStretch();
    
    return inputLayout;
}

QFrame* PlayerListView::setupTableSection() {
    QFrame* tableFrame = new QFrame(this);
    tableFrame->setFrameShape(QFrame::NoFrame);
    tableFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout* tableLayout = new QVBoxLayout(tableFrame);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(10);

    tableView = new QTableView(tableFrame);
    configureTableView();
    tableLayout->addWidget(tableView, 1);

    QWidget* paginationWidget = setupPaginationSection();
    
    paginationWidget->setMinimumWidth(tableView->minimumWidth());
    paginationWidget->setMaximumWidth(tableView->maximumWidth());
    
    tableLayout->addWidget(paginationWidget, 0);
    
    return tableFrame;
}

void PlayerListView::configureTableView() {
    tableView->setObjectName("playerTable");
    tableView->setModel(model);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->setShowGrid(false);
    tableView->verticalHeader()->setVisible(false);
    tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    tableView->horizontalHeader()->setSortIndicator(2, Qt::DescendingOrder);
}

QWidget* PlayerListView::setupPaginationSection() {
    QWidget* paginationWidget = new QWidget();
    paginationWidget->setFixedHeight(50);
    QHBoxLayout* paginationLayout = new QHBoxLayout(paginationWidget);
    paginationLayout->setContentsMargins(0, 0, 0, 0);
    
    prevPageButton = new QPushButton("Previous", paginationWidget);
    prevPageButton->setObjectName("prevPageButton");
    
    QSpacerItem* leftSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    QWidget* pageInfoContainer = new QWidget(paginationWidget);
    QHBoxLayout* pageInfoLayout = new QHBoxLayout(pageInfoContainer);
    pageInfoLayout->setContentsMargins(0, 0, 0, 0);
    
    pageInfoLabel = new QLabel("Page 1", pageInfoContainer);
    pageInfoLabel->setObjectName("pageInfoLabel");
    totalPagesLabel = new QLabel("/ 1", pageInfoContainer);
    totalPagesLabel->setObjectName("totalPagesLabel");
    
    pageInfoLayout->addWidget(pageInfoLabel);
    pageInfoLayout->addWidget(totalPagesLabel);
    
    QSpacerItem* rightSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    nextPageButton = new QPushButton("Next", paginationWidget);
    nextPageButton->setObjectName("nextPageButton");
    
    paginationLayout->addWidget(prevPageButton);
    paginationLayout->addSpacerItem(leftSpacer);
    paginationLayout->addWidget(pageInfoContainer);
    paginationLayout->addSpacerItem(rightSpacer);
    paginationLayout->addWidget(nextPageButton);
    
    return paginationWidget;
}

QScrollArea* PlayerListView::setupPlayerDetailsSection() {
    QScrollArea* playerDetailsScrollArea = new QScrollArea(this);
    playerDetailsScrollArea->setWidgetResizable(true);
    playerDetailsScrollArea->setFrameShape(QFrame::NoFrame);
    playerDetailsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    playerDetailsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    playerDetailsScrollArea->setMinimumWidth(250);
    playerDetailsScrollArea->setMaximumWidth(300);
    
    playerDetailsWidget = new QWidget();
    playerDetailsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    
    QVBoxLayout* detailsLayout = new QVBoxLayout(playerDetailsWidget);
    detailsLayout->setContentsMargins(5, 5, 5, 5);
    detailsLayout->setSpacing(12);
    
    playerName = new QLabel("No Player Selected", this);
    playerName->setObjectName("playerDetailsLabel");
    playerName->setAlignment(Qt::AlignCenter);
    playerName->setWordWrap(true);

    createPlayerInfoWidgets();
    createPlayerActionButtons();
    
    QHBoxLayout* imageLayout = new QHBoxLayout();
    imageLayout->addStretch();
    imageLayout->addWidget(playerImage);
    imageLayout->addStretch();

    detailsLayout->addWidget(playerName);
    detailsLayout->addLayout(imageLayout);
    detailsLayout->addWidget(playerClub);
    detailsLayout->addWidget(playerPosition);
    detailsLayout->addWidget(playerMarketValue);
    
    detailsLayout->addWidget(addToTeamButton);
    detailsLayout->addWidget(viewHistoryButton);
    detailsLayout->addWidget(selectForCompareButton);
    detailsLayout->addWidget(compareWithSelectedButton);
    detailsLayout->addWidget(clearComparisonButton);
    
    detailsLayout->addStretch(1);
    
    playerDetailsScrollArea->setWidget(playerDetailsWidget);
    return playerDetailsScrollArea;
}

void PlayerListView::createPlayerInfoWidgets() {
    playerImage = new QLabel();
    playerImage->setObjectName("playerImage");
    playerImage->setFixedSize(200, 200);
    playerImage->setAlignment(Qt::AlignCenter);
    playerImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    playerClub = new QLabel("");
    playerClub->setObjectName("playerClub");
    playerClub->setWordWrap(true);
    playerClub->setAlignment(Qt::AlignLeft);
    
    playerPosition = new QLabel("");
    playerPosition->setObjectName("playerPosition");
    playerPosition->setWordWrap(true);
    playerPosition->setAlignment(Qt::AlignLeft);
    
    playerMarketValue = new QLabel("");
    playerMarketValue->setObjectName("playerMarketValue");
    playerMarketValue->setWordWrap(true);
    playerMarketValue->setAlignment(Qt::AlignLeft);
}

void PlayerListView::createPlayerActionButtons() {
    viewHistoryButton = new QPushButton("View Rating History", this);
    viewHistoryButton->setObjectName("viewHistoryButton");
    viewHistoryButton->setEnabled(false);
    
    addToTeamButton = new QPushButton("Add to Team", this);
    addToTeamButton->setObjectName("addToTeamButton");
    addToTeamButton->setEnabled(false);
    
    selectForCompareButton = new QPushButton("Select for Compare", this);
    selectForCompareButton->setObjectName("selectForCompareButton");
    selectForCompareButton->setEnabled(false);
    
    compareWithSelectedButton = new QPushButton("Compare with Selected", this);
    compareWithSelectedButton->setObjectName("compareWithSelectedButton");
    compareWithSelectedButton->setEnabled(false);
    compareWithSelectedButton->setVisible(false);
    
    clearComparisonButton = new QPushButton("Clear Comparison", this);
    clearComparisonButton->setObjectName("clearComparisonButton");
    clearComparisonButton->setEnabled(false);
    clearComparisonButton->setVisible(false);
}

void PlayerListView::setupAnimations() {
    setupTableAnimations();
    setupPlayerDetailsAnimations();
}

void PlayerListView::setupTableAnimations() {
    tableOpacityEffect = new QGraphicsOpacityEffect(tableView);
    tableView->setGraphicsEffect(tableOpacityEffect);
    tableOpacityEffect->setOpacity(0.0);
    
    tableOpacityAnimation = new QPropertyAnimation(tableOpacityEffect, "opacity");
    tableOpacityAnimation->setDuration(250);
    tableOpacityAnimation->setStartValue(0.0);
    tableOpacityAnimation->setEndValue(1.0);
    tableOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    tableAnimGroup = new QParallelAnimationGroup(this);
    tableAnimGroup->addAnimation(tableOpacityAnimation);
}

void PlayerListView::setupPlayerDetailsAnimations() {
    playerDetailsOpacityEffect = new QGraphicsOpacityEffect(playerDetailsWidget);
    playerDetailsWidget->setGraphicsEffect(playerDetailsOpacityEffect);
    playerDetailsOpacityEffect->setOpacity(0.0);
    
    playerDetailsOpacityAnimation = new QPropertyAnimation(playerDetailsOpacityEffect, "opacity");
    playerDetailsOpacityAnimation->setDuration(250);
    playerDetailsOpacityAnimation->setStartValue(0.0);
    playerDetailsOpacityAnimation->setEndValue(1.0);
    playerDetailsOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    playerDetailsSlideAnimation = new QPropertyAnimation(playerDetailsWidget, "pos");
    playerDetailsSlideAnimation->setDuration(300);
    playerDetailsSlideAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    playerDetailsAnimGroup = new QParallelAnimationGroup(this);
    playerDetailsAnimGroup->addAnimation(playerDetailsOpacityAnimation);
    playerDetailsAnimGroup->addAnimation(playerDetailsSlideAnimation);
}

void PlayerListView::setupConnections() {
    setupPaginationConnections();
    setupTableConnections();
    setupButtonConnections();
}

void PlayerListView::setupPaginationConnections() {
    connect(prevPageButton, &QPushButton::clicked, this, [this]() {
        if (currentPage > 0) {
            currentPage--;
            updatePagination();
            animateTable();
        }
    });
    
    connect(nextPageButton, &QPushButton::clicked, this, [this]() {
        if ((currentPage + 1) * playersPerPage < model->filteredPlayerCount()) {
            currentPage++;
            updatePagination();
            animateTable();
        }
    });
}

void PlayerListView::setupTableConnections() {
    connect(searchBox, &QLineEdit::textChanged, this, &PlayerListView::searchPlayers);
    connect(positionFilter, &QComboBox::currentTextChanged, this, &PlayerListView::filterByPosition);
    connect(tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, model, &PlayerListModel::sort);
    
    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, 
        &PlayerListView::updatePlayerDetails);
    connect(tableView, &QTableView::doubleClicked, this, &PlayerListView::showPlayerHistory);
}

void PlayerListView::setupButtonConnections() {
    connect(backButton, &QPushButton::clicked, this, &PlayerListView::backToMain);
    connect(viewHistoryButton, &QPushButton::clicked, this, &PlayerListView::showPlayerHistory);
    connect(addToTeamButton, &QPushButton::clicked, this, &PlayerListView::addPlayerToTeams);
    connect(selectForCompareButton, &QPushButton::clicked, this, &PlayerListView::selectPlayerForComparison);
    connect(compareWithSelectedButton, &QPushButton::clicked, this, &PlayerListView::compareWithSelectedPlayer);
    connect(clearComparisonButton, &QPushButton::clicked, this, &PlayerListView::clearComparisonSelection);
}

void PlayerListView::updatePagination() {
    int filteredCount = model->filteredPlayerCount();
    totalPages = (filteredCount > 0) ? ((filteredCount - 1) / playersPerPage) + 1 : 1;

    if (currentPage >= totalPages) {
        currentPage = std::max(0, totalPages - 1);
    }

    model->setPagination(currentPage * playersPerPage, playersPerPage);

    pageInfoLabel->setText(QString("Page %1").arg(currentPage + 1));
    totalPagesLabel->setText(QString("/ %1").arg(totalPages));

    prevPageButton->setEnabled(currentPage > 0);
    nextPageButton->setEnabled((currentPage + 1) < totalPages);

    tableView->reset();
}

void PlayerListView::searchPlayers(const QString& text) {
    currentPage = 0;
    model->setFilter(text);
    updatePagination();
    animateTable();
}

void PlayerListView::filterByPosition(const QString& position) {
    currentPage = 0;
    model->setPositionFilter(position == "All Positions" ? "" : position);
    updatePagination();
    animateTable();
}

void PlayerListView::filterPlayers() {
    searchPlayers(searchBox->text());
    filterByPosition(positionFilter->currentText());
}

void PlayerListView::animateTable() {
    tableOpacityEffect->setOpacity(0.0);
    tableAnimGroup->start();
}

void PlayerListView::animatePlayerDetails() {
    QPoint currentPos = playerDetailsWidget->pos();
    QPoint startPos = currentPos + QPoint(30, 0);
    
    playerDetailsSlideAnimation->setStartValue(startPos);
    playerDetailsSlideAnimation->setEndValue(currentPos);
    
    playerDetailsOpacityEffect->setOpacity(0.0);
    playerDetailsAnimGroup->start();
}

void PlayerListView::updatePlayerDetails() {
    QModelIndexList indices = tableView->selectionModel()->selectedRows();
    
    if (indices.isEmpty()) {
        playerName->setText("No Player Selected");
        playerClub->setText("");
        playerPosition->setText("");
        playerMarketValue->setText("");
        playerImage->clear();
        playerImage->setText("No Image");
        
        viewHistoryButton->setEnabled(false);
        addToTeamButton->setEnabled(false);
        selectForCompareButton->setEnabled(false);
        updateComparisonButtons();
        return;
    }
    
    QModelIndex index = indices.first();
    
    if (!index.isValid()) return;
    
    int playerId = model->data(model->index(index.row(), 0), Qt::DisplayRole).toInt();
    currentPlayerId = playerId;
    
    auto allPlayers = ratingManager.getSortedRatedPlayers();
    for (const auto& p : allPlayers) {
        if (p.first == playerId) {
            playerName->setText(QString::fromStdString(p.second.name));
            playerClub->setText("Club: " + QString::fromStdString(p.second.clubName));
            playerPosition->setText("Position: " + QString::fromStdString(p.second.position) + 
                                   " (" + QString::fromStdString(p.second.subPosition) + ")");
            playerMarketValue->setText("Market Value: €" + QString::number(p.second.marketValue / 1000000.0, 'f', 1) + "M");
            
            viewHistoryButton->setEnabled(true);
            addToTeamButton->setEnabled(true);
            selectForCompareButton->setEnabled(true);
            updateComparisonButtons();
            
            QString imageUrl = QString::fromStdString(p.second.imageUrl);
            if (imageUrl.contains(",")) {
                imageUrl = imageUrl.split(",").first().trimmed();
            }
            
            if (imageUrl.startsWith("http")) {
                fetchPlayerDetailImage(imageUrl);
            } else {
                loadLocalPlayerDetailImage(imageUrl);
            }
            
            animatePlayerDetails();
            break;
        }
    }
}

void PlayerListView::fetchPlayerDetailImage(const QString& imageUrl) {
    QNetworkReply* reply = networkManager->get(QNetworkRequest(QUrl(imageUrl)));
    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QPixmap pixmap;
            pixmap.loadFromData(reply->readAll());
            playerImage->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
        } else {
            playerImage->setText("No Image Available");
        }
        reply->deleteLater();
    });
}

void PlayerListView::loadLocalPlayerDetailImage(const QString& imageUrl) {
    QPixmap pixmap;
    if (pixmap.load(imageUrl)) {
        playerImage->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
    } else {
        playerImage->setText("No Image");
    }
}

void PlayerListView::showPlayerHistory() {
    if (currentPlayerId <= 0) return;
    
    PlayerHistoryDialog dialog(ratingManager, currentPlayerId, this);
    dialog.exec();
}

void PlayerListView::selectPlayerForComparison() {
    if (currentPlayerId <= 0) return;
    
    comparisonPlayerId = currentPlayerId;
    updateComparisonButtons();
}

void PlayerListView::compareWithSelectedPlayer() {
    if (currentPlayerId <= 0 || comparisonPlayerId <= 0 || currentPlayerId == comparisonPlayerId) return;
    
    showPlayerComparison();
}

void PlayerListView::clearComparisonSelection() {
    comparisonPlayerId = -1;
    updateComparisonButtons();
}

void PlayerListView::showPlayerComparison() {
    if (comparisonPlayerId <= 0 || currentPlayerId <= 0) return;
    
    PlayerComparisonDialog dialog(ratingManager, comparisonPlayerId, currentPlayerId, this);
    dialog.exec();
}

void PlayerListView::updateComparisonButtons() {
    if (comparisonPlayerId <= 0) {
        selectForCompareButton->setVisible(true);
        selectForCompareButton->setEnabled(currentPlayerId > 0);
        compareWithSelectedButton->setVisible(false);
        clearComparisonButton->setVisible(false);
    } else if (comparisonPlayerId == currentPlayerId) {
        selectForCompareButton->setVisible(false);
        compareWithSelectedButton->setVisible(false);
        clearComparisonButton->setVisible(true);
        clearComparisonButton->setEnabled(true);
    } else {
        selectForCompareButton->setVisible(false);
        compareWithSelectedButton->setVisible(true);
        compareWithSelectedButton->setEnabled(true);
        clearComparisonButton->setVisible(true);
        clearComparisonButton->setEnabled(true);
    }
}

void PlayerListView::addPlayerToTeams() {
    if (currentPlayerId <= 0) return;
    
    TeamSelectDialog dialog(teamManager, currentPlayerId, this);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    Player currentPlayer;
    if (!findPlayerById(currentPlayerId, currentPlayer)) {
        QMessageBox::warning(this, "Error", "Player data not found");
        return;
    }
    
    std::vector<int> selectedTeamIds = dialog.getSelectedTeamIds();
    std::set<int> initialTeamIds = getTeamsContainingPlayer(currentPlayerId);
    std::set<int> finalTeamIds(selectedTeamIds.begin(), selectedTeamIds.end());
    
    auto teamsToAdd = getTeamsToAdd(initialTeamIds, finalTeamIds);
    auto teamsToRemove = getTeamsToRemove(initialTeamIds, finalTeamIds);
    
    int addedCount = processAddPlayerToTeams(teamsToAdd, currentPlayer);
    int removedCount = removePlayerFromTeams(teamsToRemove);
    
    showResultMessage(addedCount, removedCount);
}

bool PlayerListView::findPlayerById(int playerId, Player& player) {
    std::vector<std::pair<int, Player>> allPlayers = ratingManager.getSortedRatedPlayers();
    
    for (const auto& pair : allPlayers) {
        if (pair.first == playerId) {
            player = pair.second;
            return true;
        }
    }
    
    return false;
}

std::set<int> PlayerListView::getTeamsContainingPlayer(int playerId) {
    std::set<int> teamIds;
    std::vector<Team> allTeams = teamManager.getAllTeams();
    
    for (const auto& team : allTeams) {
        for (const auto& player : team.players) {
            if (player.playerId == playerId) {
                teamIds.insert(team.teamId);
                break;
            }
        }
    }
    
    return teamIds;
}

std::vector<int> PlayerListView::getTeamsToAdd(const std::set<int>& initialTeamIds, const std::set<int>& finalTeamIds) {
    std::vector<int> teamsToAdd;
    
    for (int teamId : finalTeamIds) {
        if (initialTeamIds.find(teamId) == initialTeamIds.end()) {
            teamsToAdd.push_back(teamId);
        }
    }
    
    return teamsToAdd;
}

std::vector<int> PlayerListView::getTeamsToRemove(const std::set<int>& initialTeamIds, const std::set<int>& finalTeamIds) {
    std::vector<int> teamsToRemove;
    
    for (int teamId : initialTeamIds) {
        if (finalTeamIds.find(teamId) == finalTeamIds.end()) {
            teamsToRemove.push_back(teamId);
        }
    }
    
    return teamsToRemove;
}

int PlayerListView::processAddPlayerToTeams(const std::vector<int>& teamIds, const Player& player) {
    int addedCount = 0;
    
    for (int teamId : teamIds) {
        try {
            teamManager.addPlayerToTeam(teamId, player);
            Team& team = teamManager.loadTeam(teamId);
            teamManager.saveTeamPlayers(team);
            addedCount++;
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", 
                QString("Failed to add player to team: %1").arg(e.what()));
        }
    }
    
    return addedCount;
}

int PlayerListView::removePlayerFromTeams(const std::vector<int>& teamIds) {
    int removedCount = 0;
    
    for (int teamId : teamIds) {
        try {
            teamManager.removePlayerFromTeam(teamId, currentPlayerId);
            Team& team = teamManager.loadTeam(teamId);
            teamManager.saveTeamPlayers(team);
            removedCount++;
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", 
                QString("Failed to remove player from team: %1").arg(e.what()));
        }
    }
    
    return removedCount;
}

void PlayerListView::showResultMessage(int addedCount, int removedCount) {
    if (addedCount > 0 || removedCount > 0) {
        QString message;
        if (addedCount > 0 && removedCount > 0) {
            message = QString("Player added to %1 team(s) and removed from %2 team(s)")
                .arg(addedCount).arg(removedCount);
        } else if (addedCount > 0) {
            message = QString("Player added to %1 team(s)").arg(addedCount);
        } else {
            message = QString("Player removed from %1 team(s)").arg(removedCount);
        }
        QMessageBox::information(this, "Success", message);
    } else {
        QMessageBox::information(this, "No Changes", "No changes were made to any teams");
    }
}

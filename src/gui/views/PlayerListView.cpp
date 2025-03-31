#include "gui/views/PlayerListView.h"
#include "gui/models/PlayerListModel.h"
#include "gui/components/dialogs/PlayerHistoryDialog.h"
#include "gui/components/dialogs/PlayerComparisonDialog.h"
#include "gui/components/dialogs/TeamSelectDialog.h"
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
#include <QPointer>
#include <algorithm>
#include <stdexcept>
#include <ranges>

PlayerListView::PlayerListView(RatingManager& ratingManager, TeamManager& teamManager, QWidget* parent)
    : QWidget(parent)
    , m_ratingManager(ratingManager)
    , m_teamManager(teamManager)
    , m_model(std::make_unique<PlayerListModel>(ratingManager.getSortedRatedPlayers()))
    , m_networkManager(std::make_unique<QNetworkAccessManager>(this))
{
    setupUi();
    setupAnimations();
    setupConnections();
    updatePagination();
    animateTable();
}

PlayerListView::~PlayerListView() {
    if (m_tableView) {
        disconnect(m_tableView, nullptr, nullptr, nullptr);
        if (m_tableView->selectionModel()) {
            disconnect(m_tableView->selectionModel(), nullptr, nullptr, nullptr);
        }
    }
    
    disconnect(this, nullptr, nullptr, nullptr);
    
    if (m_tableAnimGroup) {
        m_tableAnimGroup->stop();
    }
    
    if (m_playerDetailsAnimGroup) {
        m_playerDetailsAnimGroup->stop();
    }
    
    if (m_tableView && m_tableOpacityEffect) {
        m_tableView->setGraphicsEffect(nullptr);
    }
    
    if (m_playerDetailsWidget && m_playerDetailsOpacityEffect) {
        m_playerDetailsWidget->setGraphicsEffect(nullptr);
    }
    
    if (m_networkManager) {
        QList<QNetworkReply*> replies = m_networkManager->findChildren<QNetworkReply*>();
        for (auto* reply : replies) {
            if (reply) {
                reply->abort();
                reply->deleteLater();
            }
        }
    }
}

void PlayerListView::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    m_backButton = new QPushButton("Back to Menu", this);
    mainLayout->addWidget(m_backButton, 0, Qt::AlignLeft);

    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);
    
    auto* leftSideLayout = new QVBoxLayout();
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
    m_searchBox = new QLineEdit(this);
    m_searchBox->setObjectName("searchBox");
    m_searchBox->setFixedWidth(240);
    m_searchBox->setPlaceholderText("Search players");

    m_positionFilter = new QComboBox(this);
    m_positionFilter->setObjectName("positionFilter");
    m_positionFilter->addItem("All Positions");
    m_positionFilter->addItems({"Goalkeeper", "Defender", "Midfield", "Attack"});
    m_positionFilter->setFixedWidth(240);
}

QHBoxLayout* PlayerListView::createInputLayout() {
    auto* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(0, 0, 0, 0);

    auto* searchLayout = new QHBoxLayout();
    auto* searchLabel = new QLabel("Search:", this);
    searchLabel->setObjectName("searchLabel");
    searchLabel->setProperty("section", "search");
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchBox);
    searchLayout->addStretch();

    auto* filterLayout = new QHBoxLayout();
    auto* positionLabel = new QLabel("Position:", this);
    positionLabel->setObjectName("positionLabel");
    positionLabel->setProperty("section", "position");
    filterLayout->addWidget(positionLabel);
    filterLayout->addWidget(m_positionFilter);
    filterLayout->addStretch();

    inputLayout->addLayout(searchLayout);
    inputLayout->addSpacing(20);
    inputLayout->addLayout(filterLayout);
    inputLayout->addStretch();
    
    return inputLayout;
}

QFrame* PlayerListView::setupTableSection() {
    auto* tableFrame = new QFrame(this);
    tableFrame->setFrameShape(QFrame::NoFrame);
    tableFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    auto* tableLayout = new QVBoxLayout(tableFrame);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(10);

    m_tableView = new QTableView(tableFrame);
    configureTableView();
    tableLayout->addWidget(m_tableView, 1);

    QWidget* paginationWidget = setupPaginationSection();
    
    paginationWidget->setMinimumWidth(m_tableView->minimumWidth());
    paginationWidget->setMaximumWidth(m_tableView->maximumWidth());
    
    tableLayout->addWidget(paginationWidget, 0);
    
    return tableFrame;
}

void PlayerListView::configureTableView() {
    m_tableView->setObjectName("playerTable");
    m_tableView->setModel(m_model.get());
    m_tableView->setSortingEnabled(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->setShowGrid(false);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_tableView->horizontalHeader()->setSortIndicator(2, Qt::DescendingOrder);
}

QWidget* PlayerListView::setupPaginationSection() {
    QWidget* paginationWidget = new QWidget(this);
    paginationWidget->setFixedHeight(50);
    m_paginationLayout = new QHBoxLayout(paginationWidget);
    m_paginationLayout->setContentsMargins(0, 0, 0, 0);
    
    m_prevPageButton = new QPushButton("Previous", paginationWidget);
    m_prevPageButton->setObjectName("prevPageButton");
    
    QWidget* pageInfoContainer = createPageInfoContainer();
    
    m_nextPageButton = new QPushButton("Next", paginationWidget);
    m_nextPageButton->setObjectName("nextPageButton");
    
    auto* leftSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto* rightSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    
    m_paginationLayout->addWidget(m_prevPageButton);
    m_paginationLayout->addSpacerItem(leftSpacer);
    m_paginationLayout->addWidget(pageInfoContainer);
    m_paginationLayout->addSpacerItem(rightSpacer);
    m_paginationLayout->addWidget(m_nextPageButton);
    
    return paginationWidget;
}

QWidget* PlayerListView::createPageInfoContainer() {
    auto* pageInfoContainer = new QWidget(this);
    auto* pageInfoLayout = new QHBoxLayout(pageInfoContainer);
    pageInfoLayout->setContentsMargins(0, 0, 0, 0);
    
    m_pageInfoLabel = new QLabel("Page 1", pageInfoContainer);
    m_pageInfoLabel->setObjectName("pageInfoLabel");
    m_totalPagesLabel = new QLabel("/ 1", pageInfoContainer);
    m_totalPagesLabel->setObjectName("totalPagesLabel");
    
    pageInfoLayout->addWidget(m_pageInfoLabel);
    pageInfoLayout->addWidget(m_totalPagesLabel);
    
    return pageInfoContainer;
}

QScrollArea* PlayerListView::setupPlayerDetailsSection() {
    auto* playerDetailsScrollArea = new QScrollArea(this);
    playerDetailsScrollArea->setWidgetResizable(true);
    playerDetailsScrollArea->setFrameShape(QFrame::NoFrame);
    playerDetailsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    playerDetailsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    playerDetailsScrollArea->setMinimumWidth(250);
    playerDetailsScrollArea->setMaximumWidth(300);
    
    m_playerDetailsWidget = new QWidget(this);
    m_playerDetailsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    
    setupPlayerDetailsContent();
    
    playerDetailsScrollArea->setWidget(m_playerDetailsWidget);
    return playerDetailsScrollArea;
}

void PlayerListView::setupPlayerDetailsContent() {
    auto* detailsLayout = new QVBoxLayout(m_playerDetailsWidget);
    detailsLayout->setContentsMargins(5, 5, 5, 5);
    detailsLayout->setSpacing(12);
    
    createPlayerName(detailsLayout);
    createPlayerInfoWidgets();
    createPlayerActionButtons();
    
    addPlayerInfoToLayout(detailsLayout);
    addPlayerButtonsToLayout(detailsLayout);
    
    detailsLayout->addStretch(1);
}

void PlayerListView::createPlayerName(QVBoxLayout* layout) {
    m_playerName = new QLabel("No Player Selected", this);
    m_playerName->setObjectName("playerDetailsLabel");
    m_playerName->setAlignment(Qt::AlignCenter);
    m_playerName->setWordWrap(true);
    
    layout->addWidget(m_playerName);
}

void PlayerListView::addPlayerInfoToLayout(QVBoxLayout* layout) {
    auto* imageLayout = new QHBoxLayout();
    imageLayout->addStretch();
    imageLayout->addWidget(m_playerImage);
    imageLayout->addStretch();

    layout->addLayout(imageLayout);
    layout->addWidget(m_playerClub);
    layout->addWidget(m_playerPosition);
    layout->addWidget(m_playerMarketValue);
}

void PlayerListView::addPlayerButtonsToLayout(QVBoxLayout* layout) {
    layout->addWidget(m_addToTeamButton);
    layout->addWidget(m_viewHistoryButton);
    layout->addWidget(m_selectForCompareButton);
    layout->addWidget(m_compareWithSelectedButton);
    layout->addWidget(m_clearComparisonButton);
}

void PlayerListView::createPlayerInfoWidgets() {
    m_playerImage = new QLabel(this);
    m_playerImage->setObjectName("playerImage");
    m_playerImage->setFixedSize(200, 200);
    m_playerImage->setAlignment(Qt::AlignCenter);
    m_playerImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_playerClub = new QLabel("", this);
    m_playerClub->setObjectName("playerClub");
    m_playerClub->setWordWrap(true);
    m_playerClub->setAlignment(Qt::AlignLeft);
    
    m_playerPosition = new QLabel("", this);
    m_playerPosition->setObjectName("playerPosition");
    m_playerPosition->setWordWrap(true);
    m_playerPosition->setAlignment(Qt::AlignLeft);
    
    m_playerMarketValue = new QLabel("", this);
    m_playerMarketValue->setObjectName("playerMarketValue");
    m_playerMarketValue->setWordWrap(true);
    m_playerMarketValue->setAlignment(Qt::AlignLeft);
}

void PlayerListView::createPlayerActionButtons() {
    m_viewHistoryButton = new QPushButton("View Rating History", this);
    m_viewHistoryButton->setObjectName("viewHistoryButton");
    m_viewHistoryButton->setEnabled(false);
    
    m_addToTeamButton = new QPushButton("Add to Team", this);
    m_addToTeamButton->setObjectName("addToTeamButton");
    m_addToTeamButton->setEnabled(false);
    
    m_selectForCompareButton = new QPushButton("Select for Compare", this);
    m_selectForCompareButton->setObjectName("selectForCompareButton");
    m_selectForCompareButton->setEnabled(false);
    
    m_compareWithSelectedButton = new QPushButton("Compare with Selected", this);
    m_compareWithSelectedButton->setObjectName("compareWithSelectedButton");
    m_compareWithSelectedButton->setEnabled(false);
    m_compareWithSelectedButton->setVisible(false);
    
    m_clearComparisonButton = new QPushButton("Clear Comparison", this);
    m_clearComparisonButton->setObjectName("clearComparisonButton");
    m_clearComparisonButton->setEnabled(false);
    m_clearComparisonButton->setVisible(false);
}

void PlayerListView::setupAnimations() {
    setupTableAnimations();
    setupPlayerDetailsAnimations();
}

void PlayerListView::setupTableAnimations() {
    m_tableOpacityEffect = new QGraphicsOpacityEffect(m_tableView);
    m_tableView->setGraphicsEffect(m_tableOpacityEffect);
    m_tableOpacityEffect->setOpacity(0.0);
    
    m_tableOpacityAnimation = new QPropertyAnimation(m_tableOpacityEffect, "opacity", this);
    m_tableOpacityAnimation->setDuration(250);
    m_tableOpacityAnimation->setStartValue(0.0);
    m_tableOpacityAnimation->setEndValue(1.0);
    m_tableOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    m_tableAnimGroup = new QParallelAnimationGroup(this);
    m_tableAnimGroup->addAnimation(m_tableOpacityAnimation);
}

void PlayerListView::setupPlayerDetailsAnimations() {
    m_playerDetailsOpacityEffect = new QGraphicsOpacityEffect(m_playerDetailsWidget);
    m_playerDetailsWidget->setGraphicsEffect(m_playerDetailsOpacityEffect);
    m_playerDetailsOpacityEffect->setOpacity(0.0);
    
    m_playerDetailsOpacityAnimation = new QPropertyAnimation(m_playerDetailsOpacityEffect, "opacity", this);
    m_playerDetailsOpacityAnimation->setDuration(250);
    m_playerDetailsOpacityAnimation->setStartValue(0.0);
    m_playerDetailsOpacityAnimation->setEndValue(1.0);
    m_playerDetailsOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    m_playerDetailsSlideAnimation = new QPropertyAnimation(m_playerDetailsWidget, "pos", this);
    m_playerDetailsSlideAnimation->setDuration(300);
    m_playerDetailsSlideAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    m_playerDetailsAnimGroup = new QParallelAnimationGroup(this);
    m_playerDetailsAnimGroup->addAnimation(m_playerDetailsOpacityAnimation);
    m_playerDetailsAnimGroup->addAnimation(m_playerDetailsSlideAnimation);
}

void PlayerListView::setupConnections() {
    setupPaginationConnections();
    setupTableConnections();
    setupButtonConnections();
}

void PlayerListView::setupPaginationConnections() {
    connect(m_prevPageButton, &QPushButton::clicked, this, [this]() {
        if (m_currentPage > 0) {
            m_currentPage--;
            updatePagination();
            animateTable();
        }
    });
    
    connect(m_nextPageButton, &QPushButton::clicked, this, [this]() {
        if ((m_currentPage + 1) * m_playersPerPage < m_model->filteredPlayerCount()) {
            m_currentPage++;
            updatePagination();
            animateTable();
        }
    });
}

void PlayerListView::setupTableConnections() {
    connect(m_searchBox, &QLineEdit::textChanged, this, &PlayerListView::searchPlayers);
    connect(m_positionFilter, &QComboBox::currentTextChanged, this, &PlayerListView::filterByPosition);
    connect(m_tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, m_model.get(), &PlayerListModel::sort);
    
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, 
        [this](const QItemSelection&, const QItemSelection&) {
            updatePlayerDetails();
        });
    connect(m_tableView, &QTableView::doubleClicked, this, &PlayerListView::showPlayerHistory);
}

void PlayerListView::setupButtonConnections() {
    connect(m_backButton, &QPushButton::clicked, this, &PlayerListView::navigateToMainView);
    connect(m_viewHistoryButton, &QPushButton::clicked, this, &PlayerListView::showPlayerHistory);
    connect(m_addToTeamButton, &QPushButton::clicked, this, &PlayerListView::addPlayerToTeams);
    connect(m_selectForCompareButton, &QPushButton::clicked, this, &PlayerListView::selectPlayerForComparison);
    connect(m_compareWithSelectedButton, &QPushButton::clicked, this, &PlayerListView::compareWithSelectedPlayer);
    connect(m_clearComparisonButton, &QPushButton::clicked, this, &PlayerListView::clearComparisonSelection);
}

void PlayerListView::updatePagination() {
    int filteredCount = m_model->filteredPlayerCount();
    m_totalPages = (filteredCount > 0) ? ((filteredCount - 1) / m_playersPerPage) + 1 : 1;

    if (m_currentPage >= m_totalPages) {
        m_currentPage = std::max(0, m_totalPages - 1);
    }

    m_model->setPagination(m_currentPage * m_playersPerPage, m_playersPerPage);

    m_pageInfoLabel->setText(QString("Page %1").arg(m_currentPage + 1));
    m_totalPagesLabel->setText(QString("/ %1").arg(m_totalPages));

    m_prevPageButton->setEnabled(m_currentPage > 0);
    m_nextPageButton->setEnabled((m_currentPage + 1) < m_totalPages);

    m_tableView->reset();
}

void PlayerListView::searchPlayers(const QString& text) {
    m_currentPage = 0;
    m_model->setFilter(text);
    updatePagination();
    animateTable();
}

void PlayerListView::filterByPosition(const QString& position) {
    m_currentPage = 0;
    m_model->setPositionFilter(position == "All Positions" ? "" : position);
    updatePagination();
    animateTable();
}

void PlayerListView::filterPlayers() {
    searchPlayers(m_searchBox->text());
    filterByPosition(m_positionFilter->currentText());
}

void PlayerListView::animateTable() {
    m_tableOpacityEffect->setOpacity(0.0);
    m_tableAnimGroup->start();
}

void PlayerListView::animatePlayerDetails() {
    QPoint currentPos = m_playerDetailsWidget->pos();
    QPoint startPos = currentPos + QPoint(30, 0);
    
    m_playerDetailsSlideAnimation->setStartValue(startPos);
    m_playerDetailsSlideAnimation->setEndValue(currentPos);
    
    m_playerDetailsOpacityEffect->setOpacity(0.0);
    m_playerDetailsAnimGroup->start();
}

void PlayerListView::handleTableSelectionChanged() {
    QModelIndexList indices = m_tableView->selectionModel()->selectedRows();
    
    if (indices.isEmpty() || !indices.first().isValid()) {
        clearPlayerDetails();
        return;
    }
    
    QModelIndex index = indices.first();
    int playerId = m_model->data(m_model->index(index.row(), 0), Qt::DisplayRole).toInt();
    m_currentPlayerId = playerId;
    
    findAndUpdatePlayerDetails(playerId);
}

void PlayerListView::clearPlayerDetails() {
    m_playerName->setText("No Player Selected");
    m_playerClub->setText("");
    m_playerPosition->setText("");
    m_playerMarketValue->setText("");
    m_playerImage->clear();
    m_playerImage->setText("No Image");
    
    m_viewHistoryButton->setEnabled(false);
    m_addToTeamButton->setEnabled(false);
    m_selectForCompareButton->setEnabled(false);
    updateComparisonButtons();
}

void PlayerListView::findAndUpdatePlayerDetails(int playerId) {
    const auto& allPlayers = m_ratingManager.getSortedRatedPlayers();
    for (const auto& p : allPlayers) {
        if (p.first == playerId) {
            updatePlayerDetails(p.second);
            break;
        }
    }
}

void PlayerListView::updatePlayerDetails() {
    handleTableSelectionChanged();
}

void PlayerListView::updatePlayerDetails(const Player& player) {
    m_playerName->setText(QString::fromStdString(player.name));
    m_playerClub->setText("Club: " + QString::fromStdString(player.clubName));
    m_playerPosition->setText("Position: " + QString::fromStdString(player.position) + 
                            " (" + QString::fromStdString(player.subPosition) + ")");
    
    double marketValueInMillions = player.marketValue / 1000000.0;
    m_playerMarketValue->setText("Market Value: €" + QString::number(marketValueInMillions, 'f', 1) + "M");
    
    m_viewHistoryButton->setEnabled(true);
    m_addToTeamButton->setEnabled(true);
    m_selectForCompareButton->setEnabled(true);
    
    handlePlayerImageLoading(player);
    updateComparisonButtons();
    animatePlayerDetails();
}

void PlayerListView::handlePlayerImageLoading(const Player& player) {
    QString imageUrl = QString::fromStdString(player.imageUrl);
    if (imageUrl.contains(",")) {
        imageUrl = imageUrl.split(",").first().trimmed();
    }
    
    if (imageUrl.startsWith("http")) {
        fetchPlayerDetailImage(imageUrl);
    } else {
        loadLocalPlayerDetailImage(imageUrl);
    }
}

void PlayerListView::fetchPlayerDetailImage(const QString& imageUrl) {
    QNetworkReply* reply = m_networkManager->get(QNetworkRequest(QUrl(imageUrl)));
    
    QPointer<QNetworkReply> safeReply(reply);
    
    connect(reply, &QNetworkReply::finished, this, [this, safeReply]() {
        if (!safeReply) {
            return;
        }
        
        if (safeReply->error() == QNetworkReply::NoError) {
            QPixmap pixmap;
            pixmap.loadFromData(safeReply->readAll());
            m_playerImage->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
        } else {
            m_playerImage->setText("No Image Available");
        }
        
        safeReply->deleteLater();
    });
    
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [safeReply, timer]() {
        if (safeReply && !safeReply->isFinished()) {
            safeReply->abort();
        }
        
        timer->deleteLater();
    });
    timer->start(10000);
}

void PlayerListView::loadLocalPlayerDetailImage(const QString& imageUrl) {
    QPixmap pixmap;
    if (pixmap.load(imageUrl)) {
        m_playerImage->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
    } else {
        m_playerImage->setText("No Image");
    }
}

void PlayerListView::showPlayerHistory() {
    if (m_currentPlayerId <= 0) return;
    
    PlayerHistoryDialog dialog(m_ratingManager, m_currentPlayerId, this);
    dialog.exec();
}

void PlayerListView::selectPlayerForComparison() {
    if (m_currentPlayerId <= 0) return;
    
    m_comparisonPlayerId = m_currentPlayerId;
    updateComparisonButtons();
}

void PlayerListView::compareWithSelectedPlayer() {
    if (m_currentPlayerId <= 0 || m_comparisonPlayerId <= 0 || m_currentPlayerId == m_comparisonPlayerId) return;
    
    showPlayerComparison();
}

void PlayerListView::clearComparisonSelection() {
    m_comparisonPlayerId = -1;
    updateComparisonButtons();
}

void PlayerListView::showPlayerComparison() {
    if (m_comparisonPlayerId <= 0 || m_currentPlayerId <= 0) return;
    
    PlayerComparisonDialog dialog(m_ratingManager, m_comparisonPlayerId, m_currentPlayerId, this);
    dialog.exec();
}

void PlayerListView::handleComparisonState() {
    if (m_comparisonPlayerId <= 0) {
        m_selectForCompareButton->setVisible(true);
        m_selectForCompareButton->setEnabled(m_currentPlayerId > 0);
        m_compareWithSelectedButton->setVisible(false);
        m_clearComparisonButton->setVisible(false);
        return;
    }
    
    if (m_comparisonPlayerId == m_currentPlayerId) {
        m_selectForCompareButton->setVisible(false);
        m_compareWithSelectedButton->setVisible(false);
        m_clearComparisonButton->setVisible(true);
        m_clearComparisonButton->setEnabled(true);
        return;
    }
    
    m_selectForCompareButton->setVisible(false);
    m_compareWithSelectedButton->setVisible(true);
    m_compareWithSelectedButton->setEnabled(true);
    m_clearComparisonButton->setVisible(true);
    m_clearComparisonButton->setEnabled(true);
}

void PlayerListView::updateComparisonButtons() {
    handleComparisonState();
}

void PlayerListView::addPlayerToTeams() {
    if (m_currentPlayerId <= 0) return;
    
    TeamSelectDialog dialog(m_teamManager, m_currentPlayerId, this);
    
    if (dialog.exec() != QDialog::Accepted) return;
    
    Player currentPlayer;
    if (!findPlayerById(m_currentPlayerId, currentPlayer)) {
        QMessageBox::warning(this, "Error", "Player data not found");
        return;
    }
    
    std::vector<int> selectedTeamIds = dialog.getSelectedTeamIds();
    std::set<int> initialTeamIds = getTeamsContainingPlayer(m_currentPlayerId);
    std::set<int> finalTeamIds(selectedTeamIds.begin(), selectedTeamIds.end());
    
    auto teamsToAdd = getTeamsToAdd(initialTeamIds, finalTeamIds);
    auto teamsToRemove = getTeamsToRemove(initialTeamIds, finalTeamIds);
    
    int addedCount = processAddPlayerToTeams(teamsToAdd, currentPlayer);
    int removedCount = removePlayerFromTeams(teamsToRemove);
    
    showResultMessage(addedCount, removedCount);
}

void PlayerListView::navigateToMainView() {
    emit backToMain();
}

bool PlayerListView::findPlayerById(int playerId, Player& player) const {
    static std::unordered_map<int, Player> playerCache;
    
    if (playerCache.empty()) {
        const auto& allPlayers = m_ratingManager.getSortedRatedPlayers();
        for (const auto& pair : allPlayers) {
            playerCache[pair.first] = pair.second;
        }
    }
    
    auto it = playerCache.find(playerId);
    if (it != playerCache.end()) {
        player = it->second;
        return true;
    }
    return false;
}

std::set<int> PlayerListView::getTeamsContainingPlayer(int playerId) const {
    std::set<int> teamIds;
    const std::vector<Team> allTeams = m_teamManager.getAllTeams();
    
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

std::vector<int> PlayerListView::getTeamsToAdd(
    const std::set<int>& initialTeamIds,
    const std::set<int>& finalTeamIds
) const {
    std::vector<int> teamsToAdd;
    
    for (int teamId : finalTeamIds) {
        if (initialTeamIds.find(teamId) == initialTeamIds.end()) {
            teamsToAdd.push_back(teamId);
        }
    }
    
    return teamsToAdd;
}

std::vector<int> PlayerListView::getTeamsToRemove(
    const std::set<int>& initialTeamIds,
    const std::set<int>& finalTeamIds
) const {
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
            m_teamManager.addPlayerToTeam(teamId, player);
            Team& team = m_teamManager.loadTeam(teamId);
            m_teamManager.saveTeamPlayers(team);
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
            m_teamManager.removePlayerFromTeam(teamId, m_currentPlayerId);
            Team& team = m_teamManager.loadTeam(teamId);
            m_teamManager.saveTeamPlayers(team);
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

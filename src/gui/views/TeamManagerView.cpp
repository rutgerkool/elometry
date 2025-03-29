#include "gui/views/TeamManagerView.h"
#include "gui/models/PlayerListModel.h"
#include "gui/models/TeamListModel.h"
#include "gui/components/dialogs/ClubSelectDialog.h"
#include "gui/components/dialogs/PlayerHistoryDialog.h"
#include "gui/components/dialogs/PlayerComparisonDialog.h"
#include "gui/components/dialogs/PlayerSelectDialog.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QScrollArea>
#include <QtGui/QStandardItem>
#include <QTimer>
#include <QInputDialog>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <algorithm>
#include <ranges>

TeamManagerView::TeamManagerView(TeamManager& teamManager, QWidget* parent)
    : QWidget(parent)
    , m_teamManager(teamManager)
    , m_model(new TeamListModel(teamManager, this))
    , m_currentTeam(nullptr)
    , m_networkManager(std::make_unique<QNetworkAccessManager>(this))
    , m_comparisonPlayerId(-1)
    , m_lineupView(nullptr)
{
    setupUi();
    setupAnimations();
    setupConnections();

    m_teamManager.loadTeams();
    m_model->refresh();
    updateTeamInfo();
    
    if (m_playerDetailsOpacityEffect) {
        m_playerDetailsOpacityEffect->setOpacity(0.0);
    }
    
    animateTeamView();
}

TeamManagerView::~TeamManagerView() {
    disconnect(this, nullptr, nullptr, nullptr);

    delete m_teamListOpacityAnimation;
    delete m_teamPlayersOpacityAnimation;
    delete m_playerDetailsOpacityAnimation; 
    delete m_playerDetailsSlideAnimation;
    
    if (m_playerDetailsAnimGroup) {
        m_playerDetailsAnimGroup->stop();
        delete m_playerDetailsAnimGroup;
    }
}

void TeamManagerView::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    setupTeamRatingDisplay();
    
    auto* topBarWidget = new QWidget(this);
    auto* topBarLayout = new QHBoxLayout(topBarWidget);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    
    setupTopButtonLayout(topBarLayout);
    
    auto* viewToggleWidget = new QWidget(this);
    auto* viewToggleLayout = new QHBoxLayout(viewToggleWidget);
    setupViewToggleButtons(viewToggleLayout);
    
    auto* topSectionLayout = new QHBoxLayout();
    topSectionLayout->addWidget(m_backButton);
    topSectionLayout->addStretch(1);
    topSectionLayout->addWidget(m_teamRatingWidget);
    
    mainLayout->addLayout(topSectionLayout);
    mainLayout->addWidget(viewToggleWidget);

    auto* threeColumnContainer = new QWidget(this);
    threeColumnContainer->setObjectName("threeColumnContainer");
    auto* threeColumnLayout = new QHBoxLayout(threeColumnContainer);
    threeColumnLayout->setSpacing(20);
    threeColumnLayout->setContentsMargins(0, 0, 0, 0);

    setupLeftContainerWidget();
    setupCenterContainerWidget();
    setupRightContainerWidget();
    
    threeColumnLayout->addWidget(m_leftWidget);
    threeColumnLayout->addWidget(m_centerStackedWidget);
    threeColumnLayout->addWidget(m_playerDetailsScrollArea);
    
    m_mainViewStack = new QStackedWidget(this);
    m_lineupContainer = new QWidget(this);
    auto* lineupLayout = new QVBoxLayout(m_lineupContainer);
    lineupLayout->setContentsMargins(10, 10, 10, 10);
    
    m_mainViewStack->addWidget(threeColumnContainer);
    m_mainViewStack->addWidget(m_lineupContainer);
    
    mainLayout->addWidget(m_mainViewStack, 1);
}

void TeamManagerView::setupTeamRatingDisplay() {
    m_teamRatingWidget = new QWidget(this);
    m_teamRatingWidget->setFixedWidth(200);
    
    auto* ratingLayout = new QVBoxLayout(m_teamRatingWidget);
    ratingLayout->setContentsMargins(0, 0, 0, 0);
    ratingLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    ratingLayout->setSpacing(2);
    
    auto* teamRatingTitle = new QLabel("Team Rating", m_teamRatingWidget);
    teamRatingTitle->setStyleSheet("font-size: 12px; color: #aaaaaa;");
    teamRatingTitle->setAlignment(Qt::AlignRight);
    
    m_teamRatingLabel = new QLabel("--", m_teamRatingWidget);
    m_teamRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white;");
    m_teamRatingLabel->setAlignment(Qt::AlignRight);
    
    ratingLayout->addWidget(teamRatingTitle);
    ratingLayout->addWidget(m_teamRatingLabel);
    
    m_hasInitialRating = false;
    m_initialTeamRating = 0.0;
}

void TeamManagerView::setupTopButtonLayout(QHBoxLayout* topBarLayout) {
    m_backButton = new QPushButton("Back to Menu", this);
    topBarLayout->addWidget(m_backButton, 0, Qt::AlignLeft);
    topBarLayout->addStretch(1);
}

void TeamManagerView::setupViewToggleButtons(QHBoxLayout* viewToggleLayout) {
    m_viewPlayersButton = new QPushButton("Manage Players", this);
    m_viewPlayersButton->setCheckable(true);
    m_viewPlayersButton->setChecked(true);
    m_viewPlayersButton->setEnabled(false);
    m_viewPlayersButton->setFixedWidth(150);
    
    m_viewLineupButton = new QPushButton("Manage Lineup", this);
    m_viewLineupButton->setCheckable(true);
    m_viewLineupButton->setEnabled(false);
    m_viewLineupButton->setFixedWidth(150);
    
    const QString buttonStyle = "QPushButton:checked { background-color: #3498db; color: white; }";
    m_viewPlayersButton->setStyleSheet(buttonStyle);
    m_viewLineupButton->setStyleSheet(buttonStyle);
    
    viewToggleLayout->addWidget(m_viewPlayersButton);
    viewToggleLayout->addWidget(m_viewLineupButton);
    viewToggleLayout->addStretch();
}

void TeamManagerView::setupLeftContainerWidget() {
    m_leftWidget = setupLeftPanel();
    m_leftWidget->setFixedWidth(width() / 3 - 30);
}

void TeamManagerView::setupCenterContainerWidget() {
    auto* centerContainer = new QWidget(this);
    auto* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    
    m_centerStackedWidget = new QStackedWidget(this);
    m_centerStackedWidget->setFixedWidth(width() / 3 - 30);
    
    auto* centerWidget = setupCenterPanel();
    m_lineupWidget = setupLineupPanel();
    
    m_centerStackedWidget->addWidget(centerWidget);
    m_centerStackedWidget->addWidget(m_lineupWidget);
}

void TeamManagerView::setupRightContainerWidget() {
    m_playerDetailsScrollArea = setupRightPanel();
    m_playerDetailsScrollArea->setFixedWidth(width() / 3 - 30);
}

QWidget* TeamManagerView::setupLeftPanel() {
    auto* leftWidget = new QWidget(this);
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    
    setupLeftPanelHeader(leftLayout);
    setupLeftPanelTeamList(leftLayout);
    setupLeftPanelButtons(leftLayout);
    
    leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    return leftWidget;
}

void TeamManagerView::setupLeftPanelHeader(QVBoxLayout* layout) {
    auto* teamsLabel = new QLabel("Existing Teams:", this);
    teamsLabel->setObjectName("teamsLabel");
    teamsLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(teamsLabel);
}

void TeamManagerView::setupLeftPanelTeamList(QVBoxLayout* layout) {
    m_teamList = new QListView(this);
    m_teamList->setModel(m_model);
    m_teamList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_teamList, 1);
}

void TeamManagerView::setupLeftPanelButtons(QVBoxLayout* layout) {
    auto* actionButtonsLayout = new QVBoxLayout();
    actionButtonsLayout->setSpacing(5);
    
    m_newTeamButton = new QPushButton("Create Team", this);
    m_loadTeamByIdButton = new QPushButton("Load Team from Club", this);
    m_editTeamNameButton = new QPushButton("Edit Team Name", this);
    m_deleteTeamButton = new QPushButton("Delete Team", this);
    m_deleteTeamButton->setObjectName("deleteTeamButton");

    actionButtonsLayout->addWidget(m_newTeamButton);
    actionButtonsLayout->addWidget(m_loadTeamByIdButton);
    actionButtonsLayout->addWidget(m_editTeamNameButton);
    actionButtonsLayout->addWidget(m_deleteTeamButton);

    layout->addLayout(actionButtonsLayout);
}

QWidget* TeamManagerView::setupCenterPanel() {
    auto* centerWidget = new QWidget(this);
    auto* centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(10, 10, 10, 10);
    
    setupCenterPanelHeader(centerLayout);
    setupCenterPanelTeamTable(centerLayout);
    setupCenterPanelBudget(centerLayout);
    setupCenterPanelButtons(centerLayout);
    
    centerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    return centerWidget;
}

void TeamManagerView::setupCenterPanelHeader(QVBoxLayout* layout) {
    auto* currentTeamLabel = new QLabel("Current Team:", this);
    currentTeamLabel->setObjectName("currentTeamLabel");
    currentTeamLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(currentTeamLabel);
}

void TeamManagerView::setupCenterPanelTeamTable(QVBoxLayout* layout) {
    m_currentTeamPlayers = new QTableView(this);
    m_currentTeamPlayers->setShowGrid(false);
    m_currentTeamPlayers->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_currentTeamPlayers->setSelectionMode(QAbstractItemView::SingleSelection);
    m_currentTeamPlayers->verticalHeader()->setVisible(false);
    m_currentTeamPlayers->horizontalHeader()->setVisible(false);
    m_currentTeamPlayers->setIconSize(QSize(32, 32));
    m_currentTeamPlayers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(m_currentTeamPlayers, 1);
}

void TeamManagerView::setupCenterPanelBudget(QVBoxLayout* layout) {
    auto* budgetLayout = new QHBoxLayout();
    budgetLayout->setSpacing(5);
    
    auto* budgetLabel = new QLabel("Budget (€):", this);
    
    m_budgetInput = new QSpinBox(this);
    m_budgetInput->setRange(0, 1000000000);
    m_budgetInput->setValue(20000000);
    m_budgetInput->setSingleStep(1000000);
    m_budgetInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    budgetLayout->addWidget(budgetLabel);
    budgetLayout->addWidget(m_budgetInput, 1);
    
    layout->addLayout(budgetLayout);
}

void TeamManagerView::setupCenterPanelButtons(QVBoxLayout* layout) {
    m_autoFillButton = new QPushButton("Auto-Fill Team", this);
    layout->addWidget(m_autoFillButton);
    
    auto* playerManagementLayout = new QVBoxLayout();
    playerManagementLayout->setSpacing(5);
    
    m_addPlayersButton = new QPushButton("Select Players", this);
    m_removePlayerButton = new QPushButton("Remove Player", this);
    
    playerManagementLayout->addWidget(m_addPlayersButton);
    playerManagementLayout->addWidget(m_removePlayerButton);
    
    layout->addLayout(playerManagementLayout);
}

QWidget* TeamManagerView::setupLineupPanel() {
    auto* lineupPanelWidget = new QWidget(this);
    auto* lineupLayout = new QVBoxLayout(lineupPanelWidget);
    lineupLayout->setContentsMargins(10, 10, 10, 10);
    
    auto* lineupLabel = new QLabel("Team Lineup:", this);
    lineupLabel->setObjectName("lineupLabel");
    lineupLabel->setAlignment(Qt::AlignLeft);
    
    auto* placeholderLabel = new QLabel("Select a team to manage lineup", this);
    placeholderLabel->setAlignment(Qt::AlignCenter);
    placeholderLabel->setStyleSheet("font-size: 16px; color: #888;");
    
    lineupLayout->addWidget(lineupLabel);
    lineupLayout->addWidget(placeholderLabel, 1);
    
    lineupPanelWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    return lineupPanelWidget;
}

QScrollArea* TeamManagerView::setupRightPanel() {
    initializePlayerDetailsScrollArea();
    setupPlayerDetailsWidget();

    return m_playerDetailsScrollArea;
}

void TeamManagerView::initializePlayerDetailsScrollArea() {
    m_playerDetailsScrollArea = new QScrollArea(this);
    m_playerDetailsScrollArea->setWidgetResizable(true);
    m_playerDetailsScrollArea->setFrameShape(QFrame::NoFrame);
    m_playerDetailsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_playerDetailsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_playerDetailsScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void TeamManagerView::setupPlayerDetailsWidget() {
    m_playerDetailsWidget = new QWidget(this);
    m_playerDetailsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    auto* rightLayout = new QVBoxLayout(m_playerDetailsWidget);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    
    setupPlayerDetailsHeader(rightLayout);
    setupPlayerDetailsContent(rightLayout);
    
    m_playerDetailsScrollArea->setWidget(m_playerDetailsWidget);
}

void TeamManagerView::setupPlayerDetailsHeader(QVBoxLayout* layout) {
    auto* playerDetailsLabel = new QLabel("Selected Player:", this);

    playerDetailsLabel->setObjectName("playerDetailsLabel");
    playerDetailsLabel->setAlignment(Qt::AlignLeft);

    layout->addWidget(playerDetailsLabel);
}

void TeamManagerView::setupPlayerDetailsContent(QVBoxLayout* layout) {
    createPlayerImageLabel();
    createPlayerTextLabels();
    createPlayerActionButtons();

    layout->addLayout(setupImageLayout());
    layout->addWidget(m_playerName);
    layout->addWidget(m_playerClub);
    layout->addWidget(m_playerPosition);
    layout->addWidget(m_playerMarketValue);
    layout->addWidget(m_playerRating);
    layout->addWidget(m_viewHistoryButton);
    layout->addWidget(m_selectForCompareButton);
    layout->addWidget(m_compareWithSelectedButton);
    layout->addWidget(m_clearComparisonButton);
    layout->addStretch();
}

void TeamManagerView::createPlayerImageLabel() {
    m_playerImage = new QLabel(this);
    m_playerImage->setObjectName("playerImage");
    m_playerImage->setFixedSize(175, 175);
    m_playerImage->setAlignment(Qt::AlignCenter);
    m_playerImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void TeamManagerView::createPlayerTextLabels() {
    m_playerName = createInfoLabel("playerName");
    m_playerClub = createInfoLabel("playerClub");
    m_playerPosition = createInfoLabel("playerPosition");
    m_playerMarketValue = createInfoLabel("playerMarketValue");
    m_playerRating = createInfoLabel("playerRating");
}

QLabel* TeamManagerView::createInfoLabel(const QString& objectName) {
    auto* label = new QLabel("", this);

    label->setObjectName(objectName);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignLeft);

    return label;
}

void TeamManagerView::createPlayerActionButtons() {
    m_viewHistoryButton = new QPushButton("View Rating History", this);
    m_viewHistoryButton->setObjectName("viewHistoryButton");
    m_viewHistoryButton->setEnabled(false);
    
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

QHBoxLayout* TeamManagerView::setupImageLayout() {
    auto* imageLayout = new QHBoxLayout();

    imageLayout->addStretch();
    imageLayout->addWidget(m_playerImage);
    imageLayout->addStretch();

    return imageLayout;
}

void TeamManagerView::setupAnimations() {
    setupTeamListAnimations();
    setupTeamPlayersAnimations();
    setupPlayerDetailsAnimations();
}

void TeamManagerView::setupTeamListAnimations() {
    m_teamListOpacityEffect = new QGraphicsOpacityEffect(m_teamList);
    m_teamList->setGraphicsEffect(m_teamListOpacityEffect);
    m_teamListOpacityEffect->setOpacity(0.0);
    
    m_teamListOpacityAnimation = new QPropertyAnimation(m_teamListOpacityEffect, "opacity", this);
    m_teamListOpacityAnimation->setDuration(250);
    m_teamListOpacityAnimation->setStartValue(0.0);
    m_teamListOpacityAnimation->setEndValue(1.0);
    m_teamListOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void TeamManagerView::setupTeamPlayersAnimations() {
    m_teamPlayersOpacityEffect = new QGraphicsOpacityEffect(m_currentTeamPlayers);
    m_currentTeamPlayers->setGraphicsEffect(m_teamPlayersOpacityEffect);
    m_teamPlayersOpacityEffect->setOpacity(0.0);
    
    m_teamPlayersOpacityAnimation = new QPropertyAnimation(m_teamPlayersOpacityEffect, "opacity", this);
    m_teamPlayersOpacityAnimation->setDuration(250);
    m_teamPlayersOpacityAnimation->setStartValue(0.0);
    m_teamPlayersOpacityAnimation->setEndValue(1.0);
    m_teamPlayersOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void TeamManagerView::setupPlayerDetailsAnimations() {
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

void TeamManagerView::animateTeamView() {
    m_teamListOpacityAnimation->start();

    QTimer::singleShot(200, [this]() {
        m_teamPlayersOpacityAnimation->start();
    });
}

void TeamManagerView::animatePlayerDetails() {
    QPoint currentPos = m_playerDetailsWidget->pos();
    QPoint startPos = currentPos + QPoint(30, 0);
    
    m_playerDetailsSlideAnimation->setStartValue(startPos);
    m_playerDetailsSlideAnimation->setEndValue(currentPos);
    
    m_playerDetailsOpacityEffect->setOpacity(0.0);
    m_playerDetailsAnimGroup->start();
}

void TeamManagerView::setupConnections() {
    setupActionConnections();
    setupSelectionConnections();
    setupComparisonConnections();
    
    disableTeamControls();

    connect(m_viewPlayersButton, &QPushButton::clicked, this, &TeamManagerView::switchToPlayersView);
    connect(m_viewLineupButton, &QPushButton::clicked, this, &TeamManagerView::switchToLineupView);
}

void TeamManagerView::switchToPlayersView() {
    if (!m_viewPlayersButton->isChecked()) {
        m_viewPlayersButton->setChecked(true);
    }
    m_viewLineupButton->setChecked(false);
    
    m_mainViewStack->setCurrentIndex(0);
    m_centerStackedWidget->setCurrentIndex(0);
}

void TeamManagerView::switchToLineupView() {
    if (!m_viewLineupButton->isChecked()) {
        m_viewLineupButton->setChecked(true);
    }

    m_viewPlayersButton->setChecked(false);
    
    if (!m_lineupView) {
        m_lineupView = new LineupView(m_teamManager, m_currentTeam, m_lineupContainer);
        
        connect(m_lineupView, &LineupView::playerClicked, 
                this, &TeamManagerView::showLineupPlayerHistory);
        
        auto* containerLayout = qobject_cast<QVBoxLayout*>(m_lineupContainer->layout());

        if (containerLayout) {
            containerLayout->addWidget(m_lineupView);
        }
    }
    
    if (m_currentTeam && m_lineupView) {
        m_lineupView->setTeam(m_currentTeam);
    }
    
    m_mainViewStack->setCurrentIndex(1);
}

void TeamManagerView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updatePanelSizes();
}

void TeamManagerView::updatePanelSizes() {
    int sectionWidth = width() / 3 - 30;
    
    m_leftWidget->setFixedWidth(sectionWidth);
    m_centerStackedWidget->setFixedWidth(sectionWidth);
    m_playerDetailsScrollArea->setFixedWidth(sectionWidth);
}

void TeamManagerView::setupActionConnections() {
    connect(m_newTeamButton, &QPushButton::clicked, this, &TeamManagerView::createNewTeam);
    connect(m_loadTeamByIdButton, &QPushButton::clicked, this, 
            &TeamManagerView::showClubSelectionDialog);
    connect(m_autoFillButton, &QPushButton::clicked, this, &TeamManagerView::autoFillTeam);
    connect(m_budgetInput, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &TeamManagerView::updateBudget);
    connect(m_removePlayerButton, &QPushButton::clicked, this, 
            &TeamManagerView::removeSelectedPlayer);
    connect(m_backButton, &QPushButton::clicked, this, &TeamManagerView::navigateBack);
    connect(m_deleteTeamButton, &QPushButton::clicked, this, 
            &TeamManagerView::deleteSelectedTeam);
    connect(m_editTeamNameButton, &QPushButton::clicked, this, &TeamManagerView::editTeamName);
    connect(m_viewHistoryButton, &QPushButton::clicked, this, 
            &TeamManagerView::showPlayerHistory);
    connect(m_addPlayersButton, &QPushButton::clicked, this, 
            &TeamManagerView::searchAndAddPlayers);
}

void TeamManagerView::setupSelectionConnections() {
    if (m_teamList->selectionModel()) { 
        connect(m_teamList->selectionModel(), &QItemSelectionModel::currentChanged, 
                this, &TeamManagerView::loadSelectedTeam);
    }
}

void TeamManagerView::setupComparisonConnections() {
    connect(m_selectForCompareButton, &QPushButton::clicked, 
            this, &TeamManagerView::selectPlayerForComparison);
    connect(m_compareWithSelectedButton, &QPushButton::clicked, 
            this, &TeamManagerView::compareWithSelectedPlayer);
    connect(m_clearComparisonButton, &QPushButton::clicked, 
            this, &TeamManagerView::clearComparisonSelection);
}

void TeamManagerView::disableTeamControls() {
    m_currentTeamPlayers->setEnabled(false);
    m_autoFillButton->setEnabled(false);
    m_budgetInput->setEnabled(false);
    m_removePlayerButton->setEnabled(false);
    m_addPlayersButton->setEnabled(false);
    m_editTeamNameButton->setEnabled(false);
    
    m_viewPlayersButton->setEnabled(false);
    m_viewLineupButton->setEnabled(false);
}

void TeamManagerView::enableTeamControls() {
    m_currentTeamPlayers->setEnabled(true);
    m_autoFillButton->setEnabled(true);
    m_budgetInput->setEnabled(true);
    m_removePlayerButton->setEnabled(true);
    m_addPlayersButton->setEnabled(true);
    m_editTeamNameButton->setEnabled(true);
    
    m_viewPlayersButton->setEnabled(true);
    m_viewLineupButton->setEnabled(true);
}

QStandardItemModel* TeamManagerView::createPlayerModel() {
    QStandardItemModel* playerModel = initializePlayerModel();
    populatePlayerModel(playerModel);
    return playerModel;
}

QStandardItemModel* TeamManagerView::initializePlayerModel() {
    auto* playerModel = new QStandardItemModel(this);
    playerModel->setColumnCount(2);
    return playerModel;
}

void TeamManagerView::populatePlayerModel(QStandardItemModel* playerModel) {
    if (!m_currentTeam) return;

    for (const auto& player : m_currentTeam->players) {
        auto* imageItem = new QStandardItem();
        auto* nameItem = new QStandardItem(QString::fromStdString(player.name));
        
        imageItem->setEditable(false);
        nameItem->setEditable(false);
        
        imageItem->setData(player.playerId, Qt::UserRole);
        nameItem->setData(player.playerId, Qt::UserRole);
        
        processPlayerImage(player.playerId, imageItem, QString::fromStdString(player.imageUrl));
        
        playerModel->appendRow({imageItem, nameItem});
    }
}

void TeamManagerView::processPlayerImage(int playerId, QStandardItem* imageItem, const QString& imageUrl) {
    if (m_playerImageCache.contains(playerId)) {
        imageItem->setIcon(QIcon(m_playerImageCache[playerId]));
    } else if (!imageUrl.isEmpty()) {
        QString processedUrl = imageUrl;

        if (processedUrl.contains(",")) {
            processedUrl = processedUrl.split(",").first().trimmed(); 
        }
        loadPlayerImage(playerId, processedUrl);
    }
}

void TeamManagerView::configureCurrentTeamPlayers(QStandardItemModel* playerModel) {
    m_currentTeamPlayers->setModel(playerModel);
    m_currentTeamPlayers->setColumnWidth(0, 40);
    m_currentTeamPlayers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_currentTeamPlayers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    if (m_currentTeamPlayers->selectionModel()) {
        disconnect(m_currentTeamPlayers->selectionModel(), nullptr, this, nullptr);
        connect(m_currentTeamPlayers->selectionModel(), &QItemSelectionModel::currentChanged, 
                this, &TeamManagerView::updatePlayerDetails);
    }
}

void TeamManagerView::updateTeamRatingDisplay() {
    if (!m_currentTeam || m_currentTeam->players.empty()) {
        if (m_teamRatingLabel) {
            m_teamRatingLabel->setText("--");
            m_teamRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white; text-align: right;");
        }
        return;
    }

    double totalRating = 0.0;

    for (const auto& player : m_currentTeam->players) {
        totalRating += player.rating;
    }

    double averageRating = totalRating / m_currentTeam->players.size();
    
    if (m_teamInitialRatings.find(m_currentTeam->teamId) == m_teamInitialRatings.end()) {
        m_teamInitialRatings[m_currentTeam->teamId] = averageRating;
    }
    
    double initialTeamRating = m_teamInitialRatings[m_currentTeam->teamId];
    double ratingDiff = averageRating - initialTeamRating;
    
    updateTeamRatingLabel(averageRating, ratingDiff);
}

void TeamManagerView::updateTeamRatingLabel(double averageRating, double ratingDiff) {
    QString displayText;
    QString styleSheet;
    
    if (ratingDiff > 0.01) {
        displayText = QString("%1 <span style='font-size: 12px; color: #4CAF50;'>+%2</span>")
                        .arg(averageRating, 0, 'f', 1)
                        .arg(ratingDiff, 0, 'f', 1);
        styleSheet = "font-size: 14px; font-weight: bold; color: #4CAF50; text-align: right;";
    } else if (ratingDiff < -0.01) {
        displayText = QString("%1 <span style='font-size: 12px; color: #F44336;'>%2</span>")
                        .arg(averageRating, 0, 'f', 1)
                        .arg(ratingDiff, 0, 'f', 1);
        styleSheet = "font-size: 14px; font-weight: bold; color: #F44336; text-align: right;";
    } else {
        displayText = QString("%1").arg(averageRating, 0, 'f', 1);
        styleSheet = "font-size: 14px; font-weight: bold; color: white; text-align: right;";
    }
    
    m_teamRatingLabel->setText(displayText);
    m_teamRatingLabel->setStyleSheet(styleSheet);
}

void TeamManagerView::updateTeamInfo() {
    if (!m_currentTeam) {
        m_currentTeamPlayers->setModel(nullptr);
        disableTeamControls();
        
        updateTeamRatingDisplay();
        return;
    }

    QAbstractItemModel* oldModel = m_currentTeamPlayers->model();
    QStandardItemModel* playerModel = createPlayerModel();
    
    configureCurrentTeamPlayers(playerModel);
    
    if (oldModel && oldModel != playerModel) {
        delete oldModel;
    }

    enableTeamControls();
    m_budgetInput->setValue(m_currentTeam->budget);
    
    updateTeamRatingDisplay();
    
    if (m_lineupView) {
        m_lineupView->setTeam(m_currentTeam);
    }
}

Player* TeamManagerView::findPlayerById(int playerId) {
    if (!m_currentTeam) return nullptr;
    
    for (auto& player : m_currentTeam->players) {
        if (player.playerId == playerId) {
            return &player;
        }
    }

    return nullptr;
}

int TeamManagerView::getSelectedPlayerId() {
    QModelIndex index = m_currentTeamPlayers->currentIndex();

    if (!index.isValid() || !m_currentTeam) return -1;

    QModelIndex playerIdIndex = m_currentTeamPlayers->model()->index(index.row(), 0);

    return playerIdIndex.data(Qt::UserRole).toInt();
}

void TeamManagerView::updatePlayerDetails() {
    int playerId = getSelectedPlayerId();

    if (playerId <= 0) return;
    
    Player* player = findPlayerById(playerId);

    if (!player) return;
    
    updatePlayerInfoLabels(player);
    updatePlayerImage(player);
    
    animatePlayerDetails();
    
    m_viewHistoryButton->setEnabled(true);
    m_selectForCompareButton->setEnabled(true);
    updateComparisonButtons();
}

void TeamManagerView::updatePlayerInfoLabels(Player* player) {
    m_playerName->setText("Name: " + QString::fromStdString(player->name));
    m_playerClub->setText("Club: " + QString::fromStdString(player->clubName));
    m_playerPosition->setText("Position: " + QString::fromStdString(player->position));
    m_playerMarketValue->setText("Market Value: €" + QString::number(player->marketValue / 1000000) + "M");
    m_playerRating->setText("Rating: " + QString::number(player->rating));
}

void TeamManagerView::updatePlayerImage(Player* player) {
    QString imageUrl = QString::fromStdString(player->imageUrl);

    if (imageUrl.contains(",")) {
        imageUrl = imageUrl.split(",").first().trimmed(); 
    }

    if (imageUrl.startsWith("http")) {
        fetchPlayerDetailImage(imageUrl);
    } else {
        loadLocalPlayerDetailImage(imageUrl);
    }
}

void TeamManagerView::removeSelectedPlayer() {
    int playerId = getSelectedPlayerId();

    if (playerId <= 0 || !m_currentTeam) return;
    
    m_teamManager.removePlayerFromTeam(m_currentTeam->teamId, playerId);
    m_teamManager.saveTeamPlayers(*m_currentTeam);
    
    updateTeamInfo();
    
    if (m_lineupView) {
        m_lineupView->setTeam(nullptr);
        m_lineupView->setTeam(m_currentTeam);
    }

    hidePlayerDetails();
}

void TeamManagerView::editTeamName() {
    if (!validateTeamSelection()) return;

    bool ok;
    QString currentName = QString::fromStdString(m_currentTeam->teamName);
    QString newName = QInputDialog::getText(
        this, 
        "Edit Team Name", 
        "Enter new team name:", 
        QLineEdit::Normal, 
        currentName, 
        &ok
    );

    if (ok && !newName.isEmpty()) {
        processTeamNameUpdate(newName);
    }
}

bool TeamManagerView::validateTeamSelection() {
    QModelIndex index = m_teamList->currentIndex();

    if (!index.isValid() || !m_currentTeam) {
        QMessageBox::warning(this, "Error", "Please select a team first");
        return false;
    }

    return true;
}

void TeamManagerView::processTeamNameUpdate(const QString& newName) {
    try {
        int currentTeamId = m_currentTeam->teamId;

        if (m_teamManager.updateTeamName(currentTeamId, newName.toStdString())) {
            m_currentTeam = &m_teamManager.loadTeam(currentTeamId);
            refreshTeamListAndSelection(currentTeamId);
            updateTeamInfo();
        } else {
            QMessageBox::critical(this, "Error", "Failed to update team name in database");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to update team name: %1").arg(e.what())
        );
    }
}

void TeamManagerView::refreshTeamListAndSelection(int teamId) {
    m_model->refresh();
    
    for(int i = 0; i < m_model->rowCount(); i++) {
        QModelIndex idx = m_model->index(i, 0);

        if (m_model->data(idx, Qt::UserRole).toInt() == teamId) {
            m_teamList->setCurrentIndex(idx);
            break;
        }
    }
}

void TeamManagerView::loadPlayerImage(int playerId, const QString& imageUrl) {
    if (imageUrl.isEmpty()) return;
    
    if (imageUrl.startsWith("http")) {
        loadNetworkImage(playerId, imageUrl);
    } else {
        loadLocalImage(playerId, imageUrl);
    }
}

void TeamManagerView::loadNetworkImage(int playerId, const QString& imageUrl) {
    QUrl url(imageUrl);
    QNetworkRequest request(url);
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, playerId]() {
        this->handleImageResponse(reply, playerId);
        reply->deleteLater();
    });
    
    connect(reply, &QNetworkReply::errorOccurred, this, [reply]() {
        reply->deleteLater();
    });
    
    QTimer::singleShot(10000, reply, &QNetworkReply::abort);
}

void TeamManagerView::handleImageResponse(QNetworkReply* reply, int playerId) {
    if (reply->error() == QNetworkReply::NoError) {
        QPixmap pixmap;
        pixmap.loadFromData(reply->readAll());

        QPixmap scaledPixmap = pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_playerImageCache[playerId] = scaledPixmap;

        updatePlayerImageInModel(playerId);
    }
}

void TeamManagerView::loadLocalImage(int playerId, const QString& imageUrl) {
    QPixmap pixmap;

    if (pixmap.load(imageUrl)) {
        QPixmap scaledPixmap = pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_playerImageCache[playerId] = scaledPixmap;
        updatePlayerImageInModel(playerId);
    }
}

void TeamManagerView::updatePlayerImageInModel(int playerId) {
    auto* model = qobject_cast<QStandardItemModel*>(m_currentTeamPlayers->model());

    if (!model) return;
    
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex idx = model->index(row, 0);

        if (idx.data(Qt::UserRole).toInt() == playerId) {
            QStandardItem* item = model->item(row, 0);

            if (item) {
                item->setIcon(QIcon(m_playerImageCache[playerId]));
                m_currentTeamPlayers->update(idx);
            }

            break;
        }
    }
}

void TeamManagerView::fetchPlayerDetailImage(const QString& imageUrl) {
    QNetworkReply* reply = m_networkManager->get(QNetworkRequest(QUrl(imageUrl)));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handlePlayerDetailImageResponse(reply);
    });
}

void TeamManagerView::handlePlayerDetailImageResponse(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QPixmap pixmap;
        pixmap.loadFromData(reply->readAll());
        m_playerImage->setPixmap(pixmap.scaled(175, 175, Qt::KeepAspectRatio));
    } else {
        m_playerImage->setText("No Image Available");
    }

    reply->deleteLater();
}

void TeamManagerView::loadLocalPlayerDetailImage(const QString& imageUrl) {
    QPixmap pixmap;

    if (pixmap.load(imageUrl)) {
        m_playerImage->setPixmap(pixmap.scaled(175, 175, Qt::KeepAspectRatio));
    } else {
        m_playerImage->setText("No Image");
    }
}

void TeamManagerView::deleteSelectedTeam() {
    QModelIndex index = m_teamList->currentIndex();
    
    if (!index.isValid()) return;

    int teamId = m_model->data(index, Qt::UserRole).toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, 
        "Delete Team", 
        "Are you sure you want to delete this team?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        deleteTeamAndUpdateUI(teamId);
    }
}

void TeamManagerView::deleteTeamAndUpdateUI(int teamId) {
    try {
        m_teamManager.deleteTeam(teamId);
        m_model->refresh();
        m_currentTeam = nullptr;
        updateTeamInfo();
        hidePlayerDetails();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to delete team: %1").arg(e.what())
        );
    }
}

void TeamManagerView::showPlayerHistory() {
    int playerId = getSelectedPlayerId();

    if (playerId <= 0) return;
    
    PlayerHistoryDialog dialog(m_teamManager.getRatingManager(), playerId, this);
    dialog.exec();
}

void TeamManagerView::searchAndAddPlayers() {
    if (!m_currentTeam) {
        QMessageBox::warning(this, "Error", "Please select or create a team first");
        return;
    }

    PlayerSelectDialog dialog(m_teamManager, m_currentTeam, this);

    if (dialog.exec() == QDialog::Accepted) {
        updateTeamWithSelectedPlayers(dialog.getSelectedPlayers());
    }
}

void TeamManagerView::updateTeamWithSelectedPlayers(std::span<const Player> selectedPlayers) {
    m_currentTeam->players.clear();
    
    for (const auto& player : selectedPlayers) {
        m_teamManager.addPlayerToTeam(m_currentTeam->teamId, player);
    }
    
    m_teamManager.saveTeamPlayers(*m_currentTeam);
    updateTeamInfo();
    m_teamPlayersOpacityAnimation->start();
}

void TeamManagerView::selectPlayerForComparison() {
    int playerId = getSelectedPlayerId();
    if (playerId <= 0) return;
    
    m_comparisonPlayerId = playerId;
    updateComparisonButtons();
}

void TeamManagerView::compareWithSelectedPlayer() {
    int playerId = getSelectedPlayerId();
    if (playerId <= 0 || m_comparisonPlayerId <= 0 || playerId == m_comparisonPlayerId) return;
    
    showPlayerComparison();
}

void TeamManagerView::clearComparisonSelection() {
    m_comparisonPlayerId = -1;
    updateComparisonButtons();
}

void TeamManagerView::showPlayerComparison() {
    int playerId = getSelectedPlayerId();

    if (playerId <= 0 || m_comparisonPlayerId <= 0) return;
    
    PlayerComparisonDialog dialog(
        m_teamManager.getRatingManager(), 
        m_comparisonPlayerId, 
        playerId, 
        this
    );

    dialog.exec();
}

void TeamManagerView::updateComparisonButtons() {
    QModelIndex index = m_currentTeamPlayers->currentIndex();
    if (!isValidPlayerSelection(index)) {
        disableComparisonButtons();
        return;
    }

    int playerId = m_currentTeamPlayers->model()->index(index.row(), 0).data(Qt::UserRole).toInt();
    updateComparisonButtonsState(playerId);
}

bool TeamManagerView::isValidPlayerSelection(const QModelIndex& index) {
    return index.isValid() && m_currentTeam;
}

void TeamManagerView::disableComparisonButtons() {
    m_selectForCompareButton->setEnabled(false);
    m_compareWithSelectedButton->setEnabled(false);
    m_clearComparisonButton->setEnabled(false);
}

void TeamManagerView::updateComparisonButtonsState(int playerId) {
    if (m_comparisonPlayerId <= 0) {
        showNormalSelectionState(playerId);
    } else if (m_comparisonPlayerId == playerId) {
        showSamePlayerSelectionState();
    } else {
        showComparisonReadyState();
    }
}

void TeamManagerView::showNormalSelectionState(int playerId) {
    m_selectForCompareButton->setVisible(true);
    m_selectForCompareButton->setEnabled(playerId > 0);
    m_compareWithSelectedButton->setVisible(false);
    m_clearComparisonButton->setVisible(false);
}

void TeamManagerView::showSamePlayerSelectionState() {
    m_selectForCompareButton->setVisible(false);
    m_compareWithSelectedButton->setVisible(false);
    m_clearComparisonButton->setVisible(true);
    m_clearComparisonButton->setEnabled(true);
}

void TeamManagerView::showComparisonReadyState() {
    m_selectForCompareButton->setVisible(false);
    m_compareWithSelectedButton->setVisible(true);
    m_compareWithSelectedButton->setEnabled(true);
    m_clearComparisonButton->setVisible(true);
    m_clearComparisonButton->setEnabled(true);
}

void TeamManagerView::hidePlayerDetails() {
    clearPlayerDetails();
    disablePlayerDetailButtons();
    resetComparisonState();
    fadeOutPlayerDetails();
}

void TeamManagerView::clearPlayerDetails() {
    m_playerName->clear();
    m_playerClub->clear();
    m_playerPosition->clear();
    m_playerMarketValue->clear();
    m_playerRating->clear();
    m_playerImage->clear();
}

void TeamManagerView::disablePlayerDetailButtons() {
    m_viewHistoryButton->setEnabled(false);
    m_selectForCompareButton->setEnabled(false);
    m_compareWithSelectedButton->setEnabled(false);
    m_clearComparisonButton->setEnabled(false);
}

void TeamManagerView::resetComparisonState() {
    m_compareWithSelectedButton->setVisible(false);
    m_clearComparisonButton->setVisible(false);
    m_selectForCompareButton->setVisible(true);
    m_comparisonPlayerId = -1;
}

void TeamManagerView::fadeOutPlayerDetails() {
    auto* fadeOutAnimation = new QPropertyAnimation(m_playerDetailsOpacityEffect, "opacity", this);
    fadeOutAnimation->setDuration(100);
    fadeOutAnimation->setStartValue(m_playerDetailsOpacityEffect->opacity());
    fadeOutAnimation->setEndValue(0.0);
    fadeOutAnimation->setEasingCurve(QEasingCurve::InCubic);
    
    connect(fadeOutAnimation, &QPropertyAnimation::finished, 
            fadeOutAnimation, &QPropertyAnimation::deleteLater);

    fadeOutAnimation->start();
}

void TeamManagerView::showLineupPlayerHistory(int playerId) {
    if (playerId <= 0 || !m_currentTeam) return;
    
    PlayerHistoryDialog dialog(m_teamManager.getRatingManager(), playerId, this);
    dialog.exec();
}

void TeamManagerView::createNewTeam() {
    bool ok;
    QString name = QInputDialog::getText(
        this, 
        "Create New Team", 
        "Enter team name:", 
        QLineEdit::Normal,
        "", 
        &ok
    );
    
    if (!ok || name.trimmed().isEmpty()) {
        return;
    }

    createAndSaveNewTeam(name.trimmed());
}

void TeamManagerView::createAndSaveNewTeam(const QString& name) {
    try {
        Team& newTeam = m_teamManager.createTeam(name.toStdString());
        m_currentTeam = &newTeam;
        m_teamManager.saveTeam(newTeam);
        m_model->refresh();
        updateTeamInfo();
        
        enableTeamControls();
        m_teamPlayersOpacityAnimation->start();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to create team: %1").arg(e.what())
        );
        m_currentTeam = nullptr;
    }
}

void TeamManagerView::loadSelectedTeam() {
    QModelIndex index = m_teamList->currentIndex();

    if (!index.isValid()) return;

    hidePlayerDetails();
    
    m_playerImageCache.clear();
    
    m_hasInitialRating = false;
    
    loadTeamById(m_model->data(index, Qt::UserRole).toInt());
}

void TeamManagerView::loadTeamById(int teamId) {
    try {
        Team& selectedTeam = m_teamManager.loadTeam(teamId);
        m_currentTeam = &selectedTeam;
        updateTeamInfo();
        
        enableTeamControls();
        m_teamPlayersOpacityAnimation->start();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to load team: %1").arg(e.what())
        );

        m_currentTeam = nullptr;
        m_editTeamNameButton->setEnabled(false);
        m_addPlayersButton->setEnabled(false);
    }
}

void TeamManagerView::showClubSelectionDialog() {
    if (m_availableClubs.empty()) {
        loadAvailableClubs();
    }
    
    ClubSelectDialog dialog(m_availableClubs, this);

    if (dialog.exec() == QDialog::Accepted) {
        if (auto clubIdOpt = dialog.getSelectedClubId()) {
            loadTeamFromClub(*clubIdOpt);
        }
    }
}

void TeamManagerView::loadAvailableClubs() {
    try {
        m_availableClubs = m_teamManager.getAllClubs();
    } catch (const std::exception& e) {
        QMessageBox::warning(
            this, 
            "Error", 
            QString("Failed to load clubs: %1").arg(e.what())
        );
    }
}

void TeamManagerView::loadTeamFromClub(int clubId) {
    try {
        Team& selectedTeam = m_teamManager.loadTeamFromClub(clubId);
        m_currentTeam = &selectedTeam;
        m_teamManager.saveTeam(selectedTeam);
        m_model->refresh();
        updateTeamInfo();
        m_editTeamNameButton->setEnabled(true);
        m_teamPlayersOpacityAnimation->start();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to load team: %1").arg(e.what())
        );
        m_currentTeam = nullptr;
        m_editTeamNameButton->setEnabled(false);
    }
}

void TeamManagerView::autoFillTeam() {
    if (!m_currentTeam) {
        QMessageBox::warning(this, "Error", "No team loaded");
        return;
    }

    try {
        m_teamManager.autoFillTeam(*m_currentTeam, m_budgetInput->value());
        m_teamManager.saveTeamPlayers(*m_currentTeam);
        updateTeamInfo();
        m_teamPlayersOpacityAnimation->start();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to auto-fill team: %1").arg(e.what())
        );
    }
}

void TeamManagerView::updateBudget(int newBudget) {
    if (m_currentTeam) {
        m_teamManager.setTeamBudget(m_currentTeam->teamId, newBudget);
    }
}

void TeamManagerView::navigateBack() {
    emit backToMain();
}

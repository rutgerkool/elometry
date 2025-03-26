#include "gui/views/TeamManagerView.h"
#include "gui/models/PlayerListModel.h"
#include "gui/models/TeamListModel.h"
#include "gui/components/dialogs/ClubSelectDialog.h"
#include "gui/components/dialogs/PlayerHistoryDialog.h"
#include "gui/components/dialogs/PlayerComparisonDialog.h"
#include "gui/components/dialogs/PlayerSelectDialog.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QTimer>
#include <QInputDialog>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QScrollArea>
#include <iostream>

TeamManagerView::TeamManagerView(TeamManager& tm, QWidget *parent)
    : QWidget(parent)
    , teamManager(tm)
    , model(new TeamListModel(teamManager, this))
    , currentTeam(nullptr)
    , networkManager(new QNetworkAccessManager(this))
    , comparisonPlayerId(-1)
    , lineupView(nullptr)
{
    setupUi();
    setupAnimations();
    setupConnections();

    teamManager.loadTeams();
    model->refresh();
    updateTeamInfo();
    playerDetailsOpacityEffect->setOpacity(0.0);
    animateTeamView();
}

TeamManagerView::~TeamManagerView() {
    disconnect(this, nullptr, nullptr, nullptr);

    delete teamListOpacityAnimation;
    teamListOpacityAnimation = nullptr;
    delete teamPlayersOpacityAnimation;
    teamPlayersOpacityAnimation = nullptr;
    delete playerDetailsOpacityAnimation; 
    playerDetailsOpacityAnimation = nullptr;
    delete playerDetailsSlideAnimation;
    playerDetailsSlideAnimation = nullptr;
    
    if (playerDetailsAnimGroup) {
        playerDetailsAnimGroup->stop();
        delete playerDetailsAnimGroup;
        playerDetailsAnimGroup = nullptr;
    }
}

void TeamManagerView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    setupTeamRatingDisplay();
    
    QWidget* topBarWidget = new QWidget(this);
    QHBoxLayout* topBarLayout = new QHBoxLayout(topBarWidget);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    
    setupTopButtonLayout(topBarLayout);
    
    QWidget* viewToggleWidget = new QWidget(this);
    QHBoxLayout* viewToggleLayout = new QHBoxLayout(viewToggleWidget);
    setupViewToggleButtons(viewToggleLayout);
    
    QHBoxLayout* topSectionLayout = new QHBoxLayout();
    topSectionLayout->addWidget(backButton);
    topSectionLayout->addStretch(1);
    topSectionLayout->addWidget(teamRatingWidget);
    
    mainLayout->addLayout(topSectionLayout);
    mainLayout->addWidget(viewToggleWidget);

    QWidget* threeColumnContainer = new QWidget(this);
    setupThreeColumnContainer(threeColumnContainer);
    
    setupMainLayout(mainLayout, threeColumnContainer);
}

void TeamManagerView::setupTopButtonLayout(QHBoxLayout* topButtonLayout) {
    backButton = new QPushButton("Back to Menu", this);
    topButtonLayout->addWidget(backButton, 0, Qt::AlignLeft);
    topButtonLayout->addStretch(1);
}

void TeamManagerView::setupViewToggleButtons(QHBoxLayout* viewToggleLayout) {
    viewPlayersButton = new QPushButton("Manage Players", this);
    viewPlayersButton->setCheckable(true);
    viewPlayersButton->setChecked(true);
    viewPlayersButton->setEnabled(false);
    viewPlayersButton->setFixedWidth(150);
    
    viewLineupButton = new QPushButton("Manage Lineup", this);
    viewLineupButton->setCheckable(true);
    viewLineupButton->setEnabled(false);
    viewLineupButton->setFixedWidth(150);
    
    const QString buttonStyle = "QPushButton:checked { background-color: #3498db; color: white; }";
    viewPlayersButton->setStyleSheet(buttonStyle);
    viewLineupButton->setStyleSheet(buttonStyle);
    
    viewToggleLayout->addWidget(viewPlayersButton);
    viewToggleLayout->addWidget(viewLineupButton);
    viewToggleLayout->addStretch();
}

void TeamManagerView::setupMainLayout(QVBoxLayout* mainLayout, QWidget* threeColumnContainer) {
    QWidget* lineupContainer = new QWidget(this);
    QVBoxLayout* lineupContainerLayout = new QVBoxLayout(lineupContainer);
    lineupContainerLayout->setContentsMargins(0, 0, 0, 0);
    
    QStackedWidget* mainViewStack = new QStackedWidget(this);
    mainViewStack->addWidget(threeColumnContainer);
    mainViewStack->addWidget(lineupContainer);
    
    mainLayout->addWidget(mainViewStack, 1);
    
    this->mainViewStack = mainViewStack;
    this->lineupContainer = lineupContainer;
}

void TeamManagerView::setupThreeColumnContainer(QWidget* threeColumnContainer) {
    QHBoxLayout* threeColumnLayout = new QHBoxLayout(threeColumnContainer);
    threeColumnLayout->setSpacing(20);
    threeColumnLayout->setContentsMargins(0, 0, 0, 0);

    setupLeftContainerWidget(threeColumnLayout);
    setupCenterContainerWidget(threeColumnLayout);
    setupRightContainerWidget(threeColumnLayout);
}

void TeamManagerView::setupLeftContainerWidget(QHBoxLayout* layout) {
    leftWidget = setupLeftPanel();
    leftWidget->setFixedWidth(width() / 3 - 30);
    layout->addWidget(leftWidget);
}

void TeamManagerView::setupCenterContainerWidget(QHBoxLayout* layout) {
    QWidget* centerContainer = new QWidget(this);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    
    centerStackedWidget = new QStackedWidget(this);
    centerStackedWidget->setFixedWidth(width() / 3 - 30);
    
    QWidget* centerWidget = setupCenterPanel();
    lineupWidget = setupLineupPanel();
    
    centerStackedWidget->addWidget(centerWidget);
    centerStackedWidget->addWidget(lineupWidget);
    
    centerLayout->addWidget(centerStackedWidget);
    layout->addWidget(centerContainer);
}

void TeamManagerView::setupRightContainerWidget(QHBoxLayout* layout) {
    playerDetailsScrollArea = setupRightPanel();
    playerDetailsScrollArea->setFixedWidth(width() / 3 - 30);
    layout->addWidget(playerDetailsScrollArea);
}

QWidget* TeamManagerView::setupLeftPanel() {
    QWidget* leftWidget = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(10, 10, 10, 10);
    
    setupLeftPanelHeader(leftLayout);
    setupLeftPanelTeamList(leftLayout);
    setupLeftPanelButtons(leftLayout);
    
    leftWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    return leftWidget;
}

void TeamManagerView::setupLeftPanelHeader(QVBoxLayout* layout) {
    QLabel* teamsLabel = new QLabel("Existing Teams:", this);
    teamsLabel->setObjectName("teamsLabel");
    teamsLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(teamsLabel);
}

void TeamManagerView::setupLeftPanelTeamList(QVBoxLayout* layout) {
    teamList = new QListView(this);
    teamList->setModel(model);
    teamList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(teamList, 1);
    
    if (teamList->selectionModel()) { 
        connect(teamList->selectionModel(), &QItemSelectionModel::currentChanged, 
                this, &TeamManagerView::loadSelectedTeam);
    }
}

void TeamManagerView::setupLeftPanelButtons(QVBoxLayout* layout) {
    QVBoxLayout* actionButtonsLayout = new QVBoxLayout();
    actionButtonsLayout->setSpacing(5);
    
    newTeamButton = new QPushButton("Create Team", this);
    loadTeamByIdButton = new QPushButton("Load Team from Club", this);
    editTeamNameButton = new QPushButton("Edit Team Name", this);
    deleteTeamButton = new QPushButton("Delete Team", this);
    deleteTeamButton->setObjectName("deleteTeamButton");

    actionButtonsLayout->addWidget(newTeamButton);
    actionButtonsLayout->addWidget(loadTeamByIdButton);
    actionButtonsLayout->addWidget(editTeamNameButton);
    actionButtonsLayout->addWidget(deleteTeamButton);

    layout->addLayout(actionButtonsLayout);
}

QWidget* TeamManagerView::setupCenterPanel() {
    QWidget* centerWidget = new QWidget(this);
    QVBoxLayout* centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->setContentsMargins(10, 10, 10, 10);
    
    setupCenterPanelHeader(centerLayout);
    setupCenterPanelTeamTable(centerLayout);
    setupCenterPanelBudget(centerLayout);
    setupCenterPanelButtons(centerLayout);
    
    centerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    return centerWidget;
}

void TeamManagerView::setupCenterPanelHeader(QVBoxLayout* layout) {
    QLabel* currentTeamLabel = new QLabel("Current Team:", this);
    currentTeamLabel->setObjectName("currentTeamLabel");
    currentTeamLabel->setAlignment(Qt::AlignLeft);
    layout->addWidget(currentTeamLabel);
}

void TeamManagerView::setupCenterPanelTeamTable(QVBoxLayout* layout) {
    currentTeamPlayers = new QTableView(this);
    currentTeamPlayers->setShowGrid(false);
    currentTeamPlayers->setSelectionBehavior(QAbstractItemView::SelectRows);
    currentTeamPlayers->setSelectionMode(QAbstractItemView::SingleSelection);
    currentTeamPlayers->verticalHeader()->setVisible(false);
    currentTeamPlayers->horizontalHeader()->setVisible(false);
    currentTeamPlayers->setIconSize(QSize(32, 32));
    currentTeamPlayers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layout->addWidget(currentTeamPlayers, 1);
}

void TeamManagerView::setupCenterPanelBudget(QVBoxLayout* layout) {
    QHBoxLayout* budgetLayout = new QHBoxLayout();
    budgetLayout->setSpacing(5);
    
    QLabel* budgetLabel = new QLabel("Budget (€):", this);
    
    budgetInput = new QSpinBox(this);
    budgetInput->setRange(0, 1000000000);
    budgetInput->setValue(20000000);
    budgetInput->setSingleStep(1000000);
    budgetInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    budgetLayout->addWidget(budgetLabel);
    budgetLayout->addWidget(budgetInput, 1);
    
    layout->addLayout(budgetLayout);
}

void TeamManagerView::setupCenterPanelButtons(QVBoxLayout* layout) {
    autoFillButton = new QPushButton("Auto-Fill Team", this);
    layout->addWidget(autoFillButton);
    
    QVBoxLayout* playerManagementLayout = new QVBoxLayout();
    playerManagementLayout->setSpacing(5);
    
    addPlayersButton = new QPushButton("Select Players", this);
    removePlayerButton = new QPushButton("Remove Player", this);
    
    playerManagementLayout->addWidget(addPlayersButton);
    playerManagementLayout->addWidget(removePlayerButton);
    
    layout->addLayout(playerManagementLayout);
}

QWidget* TeamManagerView::setupLineupPanel() {
    QWidget* lineupPanelWidget = new QWidget(this);
    QVBoxLayout* lineupLayout = new QVBoxLayout(lineupPanelWidget);
    lineupLayout->setContentsMargins(10, 10, 10, 10);
    
    QLabel* lineupLabel = new QLabel("Team Lineup:", this);
    lineupLabel->setObjectName("lineupLabel");
    lineupLabel->setAlignment(Qt::AlignLeft);
    
    QLabel* placeholderLabel = new QLabel("Select a team to manage lineup", this);
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

    return playerDetailsScrollArea;
}

void TeamManagerView::initializePlayerDetailsScrollArea() {
    playerDetailsScrollArea = new QScrollArea(this);
    playerDetailsScrollArea->setWidgetResizable(true);
    playerDetailsScrollArea->setFrameShape(QFrame::NoFrame);
    playerDetailsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    playerDetailsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    playerDetailsScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void TeamManagerView::setupPlayerDetailsWidget() {
    playerDetailsWidget = new QWidget(this);
    playerDetailsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout* rightLayout = new QVBoxLayout(playerDetailsWidget);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    
    setupPlayerDetailsHeader(rightLayout);
    setupPlayerDetailsContent(rightLayout);
    
    playerDetailsScrollArea->setWidget(playerDetailsWidget);
}

void TeamManagerView::setupPlayerDetailsHeader(QVBoxLayout* layout) {
    QLabel* playerDetailsLabel = new QLabel("Selected Player:", this);

    playerDetailsLabel->setObjectName("playerDetailsLabel");
    playerDetailsLabel->setAlignment(Qt::AlignLeft);

    layout->addWidget(playerDetailsLabel);
}

void TeamManagerView::setupPlayerDetailsContent(QVBoxLayout* layout) {
    createPlayerImageLabel();
    createPlayerTextLabels();
    createPlayerActionButtons();

    layout->addLayout(setupImageLayout());
    layout->addWidget(playerName);
    layout->addWidget(playerClub);
    layout->addWidget(playerPosition);
    layout->addWidget(playerMarketValue);
    layout->addWidget(playerRating);
    layout->addWidget(viewHistoryButton);
    layout->addWidget(selectForCompareButton);
    layout->addWidget(compareWithSelectedButton);
    layout->addWidget(clearComparisonButton);
    layout->addStretch();
}

void TeamManagerView::createPlayerImageLabel() {
    playerImage = new QLabel(this);
    playerImage->setObjectName("playerImage");
    playerImage->setFixedSize(175, 175);
    playerImage->setAlignment(Qt::AlignCenter);
    playerImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void TeamManagerView::createPlayerTextLabels() {
    playerName = createInfoLabel("playerName");
    playerClub = createInfoLabel("playerClub");
    playerPosition = createInfoLabel("playerPosition");
    playerMarketValue = createInfoLabel("playerMarketValue");
    playerRating = createInfoLabel("playerRating");
}

QLabel* TeamManagerView::createInfoLabel(const QString& objectName) {
    QLabel* label = new QLabel("", this);

    label->setObjectName(objectName);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignLeft);

    return label;
}

void TeamManagerView::createPlayerActionButtons() {
    viewHistoryButton = new QPushButton("View Rating History", this);
    viewHistoryButton->setObjectName("viewHistoryButton");
    viewHistoryButton->setEnabled(false);
    
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

QHBoxLayout* TeamManagerView::setupImageLayout() {
    QHBoxLayout* imageLayout = new QHBoxLayout();

    imageLayout->addStretch();
    imageLayout->addWidget(playerImage);
    imageLayout->addStretch();

    return imageLayout;
}

void TeamManagerView::setupAnimations() {
    setupTeamListAnimations();
    setupTeamPlayersAnimations();
    setupPlayerDetailsAnimations();
}

void TeamManagerView::setupTeamListAnimations() {
    teamListOpacityEffect = new QGraphicsOpacityEffect(teamList);
    teamList->setGraphicsEffect(teamListOpacityEffect);
    teamListOpacityEffect->setOpacity(0.0);
    
    teamListOpacityAnimation = new QPropertyAnimation(teamListOpacityEffect, "opacity", this);
    teamListOpacityAnimation->setDuration(250);
    teamListOpacityAnimation->setStartValue(0.0);
    teamListOpacityAnimation->setEndValue(1.0);
    teamListOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void TeamManagerView::setupTeamPlayersAnimations() {
    teamPlayersOpacityEffect = new QGraphicsOpacityEffect(currentTeamPlayers);
    currentTeamPlayers->setGraphicsEffect(teamPlayersOpacityEffect);
    teamPlayersOpacityEffect->setOpacity(0.0);
    
    teamPlayersOpacityAnimation = new QPropertyAnimation(teamPlayersOpacityEffect, "opacity", this);
    teamPlayersOpacityAnimation->setDuration(250);
    teamPlayersOpacityAnimation->setStartValue(0.0);
    teamPlayersOpacityAnimation->setEndValue(1.0);
    teamPlayersOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

void TeamManagerView::setupPlayerDetailsAnimations() {
    playerDetailsOpacityEffect = new QGraphicsOpacityEffect(playerDetailsWidget);
    playerDetailsWidget->setGraphicsEffect(playerDetailsOpacityEffect);
    playerDetailsOpacityEffect->setOpacity(0.0);
    
    playerDetailsOpacityAnimation = new QPropertyAnimation(playerDetailsOpacityEffect, "opacity", this);
    playerDetailsOpacityAnimation->setDuration(250);
    playerDetailsOpacityAnimation->setStartValue(0.0);
    playerDetailsOpacityAnimation->setEndValue(1.0);
    playerDetailsOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    playerDetailsSlideAnimation = new QPropertyAnimation(playerDetailsWidget, "pos", this);
    playerDetailsSlideAnimation->setDuration(300);
    playerDetailsSlideAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    playerDetailsAnimGroup = new QParallelAnimationGroup(this);
    playerDetailsAnimGroup->addAnimation(playerDetailsOpacityAnimation);
    playerDetailsAnimGroup->addAnimation(playerDetailsSlideAnimation);
}

void TeamManagerView::animateTeamView() {
    teamListOpacityAnimation->start();

    QTimer::singleShot(200, [this]() {
        teamPlayersOpacityAnimation->start();
    });
}

void TeamManagerView::animatePlayerDetails() {
    QPoint currentPos = playerDetailsWidget->pos();
    QPoint startPos = currentPos + QPoint(30, 0);
    
    playerDetailsSlideAnimation->setStartValue(startPos);
    playerDetailsSlideAnimation->setEndValue(currentPos);
    
    playerDetailsOpacityEffect->setOpacity(0.0);
    playerDetailsAnimGroup->start();
}

void TeamManagerView::setupConnections() {
    setupActionConnections();
    setupSelectionConnections();
    setupComparisonConnections();
    
    disableTeamControls();

    connect(viewPlayersButton, &QPushButton::clicked, this, &TeamManagerView::switchToPlayersView);
    connect(viewLineupButton, &QPushButton::clicked, this, &TeamManagerView::switchToLineupView);
}

void TeamManagerView::switchToPlayersView() {
    if (!viewPlayersButton->isChecked()) {
        viewPlayersButton->setChecked(true);
    }
    viewLineupButton->setChecked(false);
    
    mainViewStack->setCurrentIndex(0);
    centerStackedWidget->setCurrentIndex(0);
}

void TeamManagerView::switchToLineupView() {
    if (!viewLineupButton->isChecked()) {
        viewLineupButton->setChecked(true);
    }

    viewPlayersButton->setChecked(false);
    
    if (!lineupView) {
        lineupView = new LineupView(teamManager, currentTeam, lineupContainer);
        
        connect(lineupView, &LineupView::playerClicked, 
                this, &TeamManagerView::showLineupPlayerHistory);
        
        QVBoxLayout* containerLayout = qobject_cast<QVBoxLayout*>(lineupContainer->layout());

        if (containerLayout) {
            containerLayout->addWidget(lineupView);
        }
    }
    
    if (currentTeam && lineupView) {
        lineupView->setTeam(currentTeam);
    }
    
    mainViewStack->setCurrentIndex(1);
}

void TeamManagerView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updatePanelSizes();
}

void TeamManagerView::updatePanelSizes() {
    int sectionWidth = width() / 3 - 30;
    
    leftWidget->setFixedWidth(sectionWidth);
    centerStackedWidget->setFixedWidth(sectionWidth);
    playerDetailsScrollArea->setFixedWidth(sectionWidth);
}

void TeamManagerView::setupActionConnections() {
    connect(newTeamButton, &QPushButton::clicked, this, &TeamManagerView::createNewTeam);
    connect(loadTeamByIdButton, &QPushButton::clicked, this, 
            &TeamManagerView::showClubSelectionDialog);
    connect(autoFillButton, &QPushButton::clicked, this, &TeamManagerView::autoFillTeam);
    connect(budgetInput, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &TeamManagerView::updateBudget);
    connect(removePlayerButton, &QPushButton::clicked, this, 
            &TeamManagerView::removeSelectedPlayer);
    connect(backButton, &QPushButton::clicked, this, &TeamManagerView::navigateBack);
    connect(deleteTeamButton, &QPushButton::clicked, this, 
            &TeamManagerView::deleteSelectedTeam);
    connect(editTeamNameButton, &QPushButton::clicked, this, &TeamManagerView::editTeamName);
    connect(viewHistoryButton, &QPushButton::clicked, this, 
            &TeamManagerView::showPlayerHistory);
    connect(addPlayersButton, &QPushButton::clicked, this, 
            &TeamManagerView::searchAndAddPlayers);
}

void TeamManagerView::setupSelectionConnections() {
    if (teamList->selectionModel()) { 
        connect(teamList->selectionModel(), &QItemSelectionModel::currentChanged, 
                this, &TeamManagerView::loadSelectedTeam);
    } else {
        qWarning() << "Warning: `teamList` selection model is null.";
    }
}

void TeamManagerView::setupComparisonConnections() {
    connect(selectForCompareButton, &QPushButton::clicked, 
            this, &TeamManagerView::selectPlayerForComparison);
    connect(compareWithSelectedButton, &QPushButton::clicked, 
            this, &TeamManagerView::compareWithSelectedPlayer);
    connect(clearComparisonButton, &QPushButton::clicked, 
            this, &TeamManagerView::clearComparisonSelection);
}

void TeamManagerView::disableTeamControls() {
    currentTeamPlayers->setEnabled(false);
    autoFillButton->setEnabled(false);
    budgetInput->setEnabled(false);
    removePlayerButton->setEnabled(false);
    addPlayersButton->setEnabled(false);
    editTeamNameButton->setEnabled(false);
    
    viewPlayersButton->setEnabled(false);
    viewLineupButton->setEnabled(false);
}

void TeamManagerView::enableTeamControls() {
    currentTeamPlayers->setEnabled(true);
    autoFillButton->setEnabled(true);
    budgetInput->setEnabled(true);
    removePlayerButton->setEnabled(true);
    addPlayersButton->setEnabled(true);
    editTeamNameButton->setEnabled(true);
    
    viewPlayersButton->setEnabled(true);
    viewLineupButton->setEnabled(true);
}

QStandardItemModel* TeamManagerView::createPlayerModel() {
    QStandardItemModel* playerModel = initializePlayerModel();

    populatePlayerModel(playerModel);

    return playerModel;
}

QStandardItemModel* TeamManagerView::initializePlayerModel() {
    QStandardItemModel* playerModel = new QStandardItemModel(this);

    playerModel->setColumnCount(2);

    return playerModel;
}

void TeamManagerView::populatePlayerModel(QStandardItemModel* playerModel) {
    if (!currentTeam) return;

    for (const auto& player : currentTeam->players) {
        QStandardItem* imageItem = new QStandardItem();
        QStandardItem* nameItem = new QStandardItem(QString::fromStdString(player.name));
        
        imageItem->setEditable(false);
        nameItem->setEditable(false);
        
        imageItem->setData(player.playerId, Qt::UserRole);
        nameItem->setData(player.playerId, Qt::UserRole);
        
        processPlayerImage(player.playerId, imageItem, QString::fromStdString(player.imageUrl));
        
        playerModel->appendRow({imageItem, nameItem});
    }
}

void TeamManagerView::processPlayerImage(int playerId, QStandardItem* imageItem, const QString& imageUrl) {
    if (playerImageCache.contains(playerId)) {
        imageItem->setIcon(QIcon(playerImageCache[playerId]));
    } else if (!imageUrl.isEmpty()) {
        QString processedUrl = imageUrl;

        if (processedUrl.contains(",")) {
            processedUrl = processedUrl.split(",").first().trimmed(); 
        }
        loadPlayerImage(playerId, processedUrl);
    }
}

void TeamManagerView::configureCurrentTeamPlayers(QStandardItemModel* playerModel) {
    currentTeamPlayers->setModel(playerModel);
    currentTeamPlayers->setColumnWidth(0, 40);
    currentTeamPlayers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    currentTeamPlayers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    if (currentTeamPlayers->selectionModel()) {
        disconnect(currentTeamPlayers->selectionModel(), nullptr, this, nullptr);
        connect(currentTeamPlayers->selectionModel(), &QItemSelectionModel::currentChanged, 
                this, &TeamManagerView::updatePlayerDetails);
    }
}

void TeamManagerView::setupTeamRatingDisplay() {
    teamRatingWidget = new QWidget(this);
    teamRatingWidget->setFixedWidth(200);
    
    QVBoxLayout* ratingLayout = new QVBoxLayout(teamRatingWidget);
    ratingLayout->setContentsMargins(0, 0, 0, 0);
    ratingLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    ratingLayout->setSpacing(2);
    
    QLabel* teamRatingTitle = new QLabel("Team Rating", teamRatingWidget);
    teamRatingTitle->setStyleSheet("font-size: 12px; color: #aaaaaa;");
    teamRatingTitle->setAlignment(Qt::AlignRight);
    
    teamRatingLabel = new QLabel("--", teamRatingWidget);
    teamRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white;");
    teamRatingLabel->setAlignment(Qt::AlignRight);
    
    ratingLayout->addWidget(teamRatingTitle);
    ratingLayout->addWidget(teamRatingLabel);
    
    hasInitialRating = false;
    initialTeamRating = 0.0;
}

void TeamManagerView::updateTeamRatingDisplay() {
    if (!currentTeam || currentTeam->players.empty()) {
        if (teamRatingLabel) {
            teamRatingLabel->setText("--");
            teamRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white; text-align: right;");
        }
        return;
    }

    double totalRating = 0.0;

    for (const auto& player : currentTeam->players) {
        totalRating += player.rating;
    }

    double averageRating = totalRating / currentTeam->players.size();
    
    if (teamInitialRatings.find(currentTeam->teamId) == teamInitialRatings.end()) {
        teamInitialRatings[currentTeam->teamId] = averageRating;
    }
    
    double initialTeamRating = teamInitialRatings[currentTeam->teamId];
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
    
    teamRatingLabel->setText(displayText);
    teamRatingLabel->setStyleSheet(styleSheet);
}

void TeamManagerView::updateTeamInfo() {
    if (!currentTeam) {
        currentTeamPlayers->setModel(nullptr);
        disableTeamControls();
        
        updateTeamRatingDisplay();
        return;
    }

    QAbstractItemModel* oldModel = currentTeamPlayers->model();
    QStandardItemModel* playerModel = createPlayerModel();
    
    configureCurrentTeamPlayers(playerModel);
    
    if (oldModel && oldModel != playerModel) {
        delete oldModel;
    }

    enableTeamControls();
    budgetInput->setValue(currentTeam->budget);
    
    updateTeamRatingDisplay();
    
    if (lineupView) {
        lineupView->setTeam(currentTeam);
    }
}

Player* TeamManagerView::findPlayerById(int playerId) {
    if (!currentTeam) return nullptr;
    
    for (auto& player : currentTeam->players) {
        if (player.playerId == playerId) {
            return &player;
        }
    }

    return nullptr;
}

int TeamManagerView::getSelectedPlayerId() {
    QModelIndex index = currentTeamPlayers->currentIndex();

    if (!index.isValid() || !currentTeam) return -1;

    QModelIndex playerIdIndex = currentTeamPlayers->model()->index(index.row(), 0);

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
    
    viewHistoryButton->setEnabled(true);
    selectForCompareButton->setEnabled(true);
    updateComparisonButtons();
}

void TeamManagerView::updatePlayerInfoLabels(Player* player) {
    playerName->setText("Name: " + QString::fromStdString(player->name));
    playerClub->setText("Club: " + QString::fromStdString(player->clubName));
    playerPosition->setText("Position: " + QString::fromStdString(player->position));
    playerMarketValue->setText("Market Value: €" + QString::number(player->marketValue / 1000000) + "M");
    playerRating->setText("Rating: " + QString::number(player->rating));
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

    if (playerId <= 0 || !currentTeam) return;
    
    teamManager.removePlayerFromTeam(currentTeam->teamId, playerId);
    teamManager.saveTeamPlayers(*currentTeam);
    
    updateTeamInfo();
    
    if (lineupView) {
        lineupView->setTeam(nullptr);
        lineupView->setTeam(currentTeam);
    }

    hidePlayerDetails();
}

void TeamManagerView::editTeamName() {
    if (!validateTeamSelection()) return;

    bool ok;
    QString currentName = QString::fromStdString(currentTeam->teamName);
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
    QModelIndex index = teamList->currentIndex();

    if (!index.isValid() || !currentTeam) {
        QMessageBox::warning(this, "Error", "Please select a team first");
        return false;
    }

    return true;
}

void TeamManagerView::processTeamNameUpdate(const QString& newName) {
    try {
        int currentTeamId = currentTeam->teamId;

        if (teamManager.updateTeamName(currentTeamId, newName.toStdString())) {
            currentTeam = &teamManager.loadTeam(currentTeamId);
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
    model->refresh();
    
    for(int i = 0; i < model->rowCount(); i++) {
        QModelIndex idx = model->index(i, 0);

        if (model->data(idx, Qt::UserRole).toInt() == teamId) {
            teamList->setCurrentIndex(idx);
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
    QNetworkReply* reply = networkManager->get(request);
    
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
        playerImageCache[playerId] = scaledPixmap;

        updatePlayerImageInModel(playerId);
    }

    reply->deleteLater();
}

void TeamManagerView::loadLocalImage(int playerId, const QString& imageUrl) {
    QPixmap pixmap;

    if (pixmap.load(imageUrl)) {
        QPixmap scaledPixmap = pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        playerImageCache[playerId] = scaledPixmap;
        updatePlayerImageInModel(playerId);
    }
}

void TeamManagerView::updatePlayerImageInModel(int playerId) {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(currentTeamPlayers->model());

    if (!model) return;
    
    for (int row = 0; row < model->rowCount(); ++row) {
        QModelIndex idx = model->index(row, 0);

        if (idx.data(Qt::UserRole).toInt() == playerId) {
            QStandardItem* item = model->item(row, 0);

            if (item) {
                item->setIcon(QIcon(playerImageCache[playerId]));
                currentTeamPlayers->update(idx);
            }

            break;
        }
    }
}

void TeamManagerView::fetchPlayerDetailImage(const QString& imageUrl) {
    QNetworkReply* reply = networkManager->get(QNetworkRequest(QUrl(imageUrl)));

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handlePlayerDetailImageResponse(reply);
    });
}

void TeamManagerView::handlePlayerDetailImageResponse(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QPixmap pixmap;
        pixmap.loadFromData(reply->readAll());
        playerImage->setPixmap(pixmap.scaled(175, 175, Qt::KeepAspectRatio));
    } else {
        playerImage->setText("No Image Available");
    }

    reply->deleteLater();
}

void TeamManagerView::loadLocalPlayerDetailImage(const QString& imageUrl) {
    QPixmap pixmap;

    if (pixmap.load(imageUrl)) {
        playerImage->setPixmap(pixmap.scaled(175, 175, Qt::KeepAspectRatio));
    } else {
        playerImage->setText("No Image");
    }
}

void TeamManagerView::deleteSelectedTeam() {
    QModelIndex index = teamList->currentIndex();
    
    if (!index.isValid()) return;

    int teamId = model->data(index, Qt::UserRole).toInt();

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
        teamManager.deleteTeam(teamId);
        model->refresh();
        currentTeam = nullptr;
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
    
    PlayerHistoryDialog dialog(teamManager.getRatingManager(), playerId, this);
    dialog.exec();
}

void TeamManagerView::searchAndAddPlayers() {
    if (!currentTeam) {
        QMessageBox::warning(this, "Error", "Please select or create a team first");
        return;
    }

    PlayerSelectDialog dialog(teamManager, currentTeam, this);

    if (dialog.exec() == QDialog::Accepted) {
        updateTeamWithSelectedPlayers(dialog.getSelectedPlayers());
    }
}

void TeamManagerView::updateTeamWithSelectedPlayers(const std::vector<Player>& selectedPlayers) {
    currentTeam->players.clear();
    
    for (const auto& player : selectedPlayers) {
        teamManager.addPlayerToTeam(currentTeam->teamId, player);
    }
    
    teamManager.saveTeamPlayers(*currentTeam);
    updateTeamInfo();
    teamPlayersOpacityAnimation->start();
}

void TeamManagerView::selectPlayerForComparison() {
    int playerId = getSelectedPlayerId();
    if (playerId <= 0) return;
    
    comparisonPlayerId = playerId;
    updateComparisonButtons();
}

void TeamManagerView::compareWithSelectedPlayer() {
    int playerId = getSelectedPlayerId();
    if (playerId <= 0 || comparisonPlayerId <= 0 || playerId == comparisonPlayerId) return;
    
    showPlayerComparison();
}

void TeamManagerView::clearComparisonSelection() {
    comparisonPlayerId = -1;
    updateComparisonButtons();
}

void TeamManagerView::showPlayerComparison() {
    int playerId = getSelectedPlayerId();

    if (playerId <= 0 || comparisonPlayerId <= 0) return;
    
    PlayerComparisonDialog dialog(
        teamManager.getRatingManager(), 
        comparisonPlayerId, 
        playerId, 
        this
    );

    dialog.exec();
}

void TeamManagerView::updateComparisonButtons() {
    QModelIndex index = currentTeamPlayers->currentIndex();
    if (!isValidPlayerSelection(index)) {
        disableComparisonButtons();
        return;
    }

    int playerId = currentTeamPlayers->model()->index(index.row(), 0).data(Qt::UserRole).toInt();
    updateComparisonButtonsState(playerId);
}

bool TeamManagerView::isValidPlayerSelection(const QModelIndex& index) {
    return index.isValid() && currentTeam;
}

void TeamManagerView::disableComparisonButtons() {
    selectForCompareButton->setEnabled(false);
    compareWithSelectedButton->setEnabled(false);
    clearComparisonButton->setEnabled(false);
}

void TeamManagerView::updateComparisonButtonsState(int playerId) {
    if (comparisonPlayerId <= 0) {
        showNormalSelectionState(playerId);
    } else if (comparisonPlayerId == playerId) {
        showSamePlayerSelectionState();
    } else {
        showComparisonReadyState();
    }
}

void TeamManagerView::showNormalSelectionState(int playerId) {
    selectForCompareButton->setVisible(true);
    selectForCompareButton->setEnabled(playerId > 0);
    compareWithSelectedButton->setVisible(false);
    clearComparisonButton->setVisible(false);
}

void TeamManagerView::showSamePlayerSelectionState() {
    selectForCompareButton->setVisible(false);
    compareWithSelectedButton->setVisible(false);
    clearComparisonButton->setVisible(true);
    clearComparisonButton->setEnabled(true);
}

void TeamManagerView::showComparisonReadyState() {
    selectForCompareButton->setVisible(false);
    compareWithSelectedButton->setVisible(true);
    compareWithSelectedButton->setEnabled(true);
    clearComparisonButton->setVisible(true);
    clearComparisonButton->setEnabled(true);
}

void TeamManagerView::hidePlayerDetails() {
    clearPlayerDetails();
    disablePlayerDetailButtons();
    resetComparisonState();
    fadeOutPlayerDetails();
}

void TeamManagerView::clearPlayerDetails() {
    playerName->clear();
    playerClub->clear();
    playerPosition->clear();
    playerMarketValue->clear();
    playerRating->clear();
    playerImage->clear();
}

void TeamManagerView::disablePlayerDetailButtons() {
    viewHistoryButton->setEnabled(false);
    selectForCompareButton->setEnabled(false);
    compareWithSelectedButton->setEnabled(false);
    clearComparisonButton->setEnabled(false);
}

void TeamManagerView::resetComparisonState() {
    compareWithSelectedButton->setVisible(false);
    clearComparisonButton->setVisible(false);
    selectForCompareButton->setVisible(true);
    comparisonPlayerId = -1;
}

void TeamManagerView::fadeOutPlayerDetails() {
    QPropertyAnimation* fadeOutAnimation = new QPropertyAnimation(playerDetailsOpacityEffect, "opacity", this);
    fadeOutAnimation->setDuration(100);
    fadeOutAnimation->setStartValue(playerDetailsOpacityEffect->opacity());
    fadeOutAnimation->setEndValue(0.0);
    fadeOutAnimation->setEasingCurve(QEasingCurve::InCubic);
    
    connect(fadeOutAnimation, &QPropertyAnimation::finished, 
            fadeOutAnimation, &QPropertyAnimation::deleteLater);

    fadeOutAnimation->start();
}

void TeamManagerView::showLineupPlayerHistory(int playerId) {
    if (playerId <= 0 || !currentTeam) return;
    
    PlayerHistoryDialog dialog(teamManager.getRatingManager(), playerId, this);
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
        Team& newTeam = teamManager.createTeam(name.toStdString());
        currentTeam = &newTeam;
        teamManager.saveTeam(newTeam);
        model->refresh();
        updateTeamInfo();
        
        enableTeamControls();
        teamPlayersOpacityAnimation->start();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to create team: %1").arg(e.what())
        );
        currentTeam = nullptr;
    }
}

void TeamManagerView::loadSelectedTeam() {
    QModelIndex index = teamList->currentIndex();

    if (!index.isValid()) return;

    hidePlayerDetails();
    
    playerImageCache.clear();
    
    hasInitialRating = false;
    
    loadTeamById(model->data(index, Qt::UserRole).toInt());
}

void TeamManagerView::loadTeamById(int teamId) {
    try {
        Team& selectedTeam = teamManager.loadTeam(teamId);
        currentTeam = &selectedTeam;
        updateTeamInfo();
        
        enableTeamControls();
        teamPlayersOpacityAnimation->start();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to load team: %1").arg(e.what())
        );

        currentTeam = nullptr;
        editTeamNameButton->setEnabled(false);
        addPlayersButton->setEnabled(false);
    }
}

void TeamManagerView::showClubSelectionDialog() {
    if (availableClubs.empty()) {
        loadAvailableClubs();
    }
    
    ClubSelectDialog dialog(availableClubs, this);

    if (dialog.exec() == QDialog::Accepted) {
        if (auto clubIdOpt = dialog.getSelectedClubId()) {
            loadTeamFromClub(*clubIdOpt);
        }
    }
}

void TeamManagerView::loadAvailableClubs() {
    try {
        availableClubs = teamManager.getAllClubs();
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
        Team& selectedTeam = teamManager.loadTeamFromClub(clubId);
        currentTeam = &selectedTeam;
        teamManager.saveTeam(selectedTeam);
        model->refresh();
        updateTeamInfo();
        editTeamNameButton->setEnabled(true);
        teamPlayersOpacityAnimation->start();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to load team: %1").arg(e.what())
        );
        currentTeam = nullptr;
        editTeamNameButton->setEnabled(false);
    }
}

void TeamManagerView::autoFillTeam() {
    if (!currentTeam) {
        QMessageBox::warning(this, "Error", "No team loaded");
        return;
    }

    try {
        teamManager.autoFillTeam(*currentTeam, budgetInput->value());
        teamManager.saveTeamPlayers(*currentTeam);
        updateTeamInfo();
        teamPlayersOpacityAnimation->start();
    } catch (const std::exception& e) {
        QMessageBox::critical(
            this, 
            "Error", 
            QString("Failed to auto-fill team: %1").arg(e.what())
        );
    }
}

void TeamManagerView::updateBudget(int newBudget) {
    if (currentTeam) {
        teamManager.setTeamBudget(currentTeam->teamId, newBudget);
    }
}

void TeamManagerView::navigateBack() {
    emit backToMain();
}

#include "gui/components/LineupView.h"
#include "gui/components/DraggableListWidget.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGroupBox>
#include <QTimer>
#include <QFile>

LineupView::LineupView(TeamManager& tm, Team* currentTeam, QWidget *parent)
    : QWidget(parent)
    , teamManager(tm)
    , currentTeam(currentTeam)
    , playerImageCache()
    , lineupRatingLabel(nullptr)
    , lineupRatingWidget(nullptr)
    , initialLineupRating(0.0)
    , hasInitialLineupRating(false)
{
    setupLineupRatingDisplay();
    setupUi();
    setupConnections();
    
    if (currentTeam) {
        setTeam(currentTeam);
    }
}

void LineupView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    
    QHBoxLayout* viewButtonsLayout = new QHBoxLayout();
    viewButtonsLayout->setContentsMargins(10, 10, 10, 0);
    mainLayout->addLayout(viewButtonsLayout);

    setupControlsLayout(mainLayout);
    setupLineupContent();
    
    mainLayout->addWidget(lineupScrollArea, 1);
    
    updateLineupVisibility(false);

    benchList->viewport()->installEventFilter(this);
    reservesList->viewport()->installEventFilter(this);
}

QWidget* LineupView::createPlaceholderWidget(QWidget* parent) {
    QWidget* placeholderWidget = new QWidget(parent);
    QVBoxLayout* placeholderLayout = new QVBoxLayout(placeholderWidget);
    
    setupPlaceholderLabels(placeholderLayout, parent);
    
    placeholderLayout->addStretch(1);
    placeholderLayout->setSpacing(10);
    
    return placeholderWidget;
}

void LineupView::setupPlaceholderLabels(QVBoxLayout* layout, QWidget* parent) {
    QLabel* noLineupLabel = new QLabel("No lineup selected", parent);
    noLineupLabel->setAlignment(Qt::AlignCenter);
    noLineupLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #666;");
    
    QLabel* instructionLabel = new QLabel("Create a new lineup or select an existing one", parent);
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("font-size: 14px; color: #888;");
    
    QFrame* line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: #444; max-width: 250px;");
    
    layout->addStretch(1);
    layout->addWidget(noLineupLabel);
    layout->addWidget(line, 0, Qt::AlignCenter);
    layout->addWidget(instructionLabel);
}

void LineupView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    
    updatePitchViewSize();
    updateListWidgetsSize();
}

void LineupView::updatePitchViewSize() {
    if (pitchView) {
        int containerWidth = lineupScrollArea->width() - 40;
        pitchView->setMinimumWidth(containerWidth * 0.65);
    }
}

void LineupView::updateListWidgetsSize() {
    if (benchList && reservesList) {
        int listWidth = lineupScrollArea->width() * 0.25;
        benchList->setMinimumWidth(listWidth);
        reservesList->setMinimumWidth(listWidth);
    }
}

void LineupView::setupControlsLayout(QVBoxLayout* mainLayout) {
    QGridLayout* controlsLayout = new QGridLayout();
    controlsLayout->setContentsMargins(10, 0, 10, 10);
    controlsLayout->setHorizontalSpacing(15);
    controlsLayout->setVerticalSpacing(10);
    
    setupLineupControls(controlsLayout);
    
    QHBoxLayout* topLayout = new QHBoxLayout();
    topLayout->addLayout(controlsLayout);
    topLayout->addStretch(1);
    topLayout->addWidget(lineupRatingWidget, 0, Qt::AlignRight | Qt::AlignTop);
    
    mainLayout->addLayout(topLayout);
}

void LineupView::setupLineupControls(QGridLayout* controlsLayout) {
    QLabel* existingLineupsLabel = new QLabel("Lineups:", this);
    existingLineupsComboBox = new QComboBox(this);
    existingLineupsComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    existingLineupsComboBox->setMinimumWidth(150);
    
    setupNewLineupButton();
    
    QHBoxLayout* dropdownAndButtonsLayout = new QHBoxLayout();
    dropdownAndButtonsLayout->setSpacing(10);
    dropdownAndButtonsLayout->addWidget(existingLineupsComboBox, 1);
    dropdownAndButtonsLayout->addSpacing(15);
    dropdownAndButtonsLayout->addWidget(newLineupButton);
    dropdownAndButtonsLayout->addWidget(deleteLineupButton);
    
    controlsLayout->addWidget(existingLineupsLabel, 0, 0);
    controlsLayout->addLayout(dropdownAndButtonsLayout, 0, 1);
}

void LineupView::setupNewLineupButton() {
    newLineupButton = new QPushButton("Create Lineup", this);
    newLineupButton->setEnabled(currentTeam != nullptr);
    deleteLineupButton = new QPushButton("Delete Lineup", this);
}

void LineupView::setupLineupContent() {
    lineupContentWidget = new QWidget(this);
    QHBoxLayout* contentLayout = new QHBoxLayout(lineupContentWidget);
    contentLayout->setContentsMargins(10, 0, 10, 10);
    
    QVBoxLayout* playersListLayout = new QVBoxLayout();
    playersListLayout->setSpacing(10);
    
    setupBenchAndReservesGroups(playersListLayout);
    setupPitchLayout(contentLayout, playersListLayout);

    setupStackedWidget();
}

void LineupView::setupStackedWidget() {
    QWidget* placeholderWidget = createPlaceholderWidget(this);
    
    lineupStackedWidget = new QStackedWidget(this);
    lineupStackedWidget->addWidget(placeholderWidget);
    lineupStackedWidget->addWidget(lineupContentWidget);
    
    lineupScrollArea = new QScrollArea(this);
    lineupScrollArea->setWidgetResizable(true);
    lineupScrollArea->setWidget(lineupStackedWidget);
    lineupScrollArea->setFrameShape(QFrame::NoFrame);
}

void LineupView::setupBenchAndReservesGroups(QVBoxLayout* playersListLayout) {
    QGroupBox* benchGroup = createBenchGroup();
    QGroupBox* reservesGroup = createReservesGroup();
    
    playersListLayout->addWidget(benchGroup);
    playersListLayout->addWidget(reservesGroup, 1);
}

QGroupBox* LineupView::createBenchGroup() {
    QGroupBox* benchGroup = new QGroupBox("Bench", this);
    QVBoxLayout* benchLayout = new QVBoxLayout(benchGroup);
    benchList = new DraggableListWidget("BENCH", this);
    setupListWidget(benchList);
    benchList->setFixedHeight(150);
    benchLayout->addWidget(benchList);
    
    return benchGroup;
}

QGroupBox* LineupView::createReservesGroup() {
    QGroupBox* reservesGroup = new QGroupBox("Reserves", this);
    QVBoxLayout* reservesLayout = new QVBoxLayout(reservesGroup);
    reservesList = new DraggableListWidget("RESERVE", this);
    setupListWidget(reservesList);
    reservesLayout->addWidget(reservesList);
    
    return reservesGroup;
}

void LineupView::setupListWidget(QListWidget* listWidget) {
    listWidget->setDragEnabled(true);
    listWidget->setAcceptDrops(true);
    listWidget->setDropIndicatorShown(true);
    listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    listWidget->setDragDropMode(QAbstractItemView::DragDrop);
    listWidget->setMinimumWidth(200);
}

void LineupView::setupPitchLayout(QHBoxLayout* contentLayout, QVBoxLayout* playersListLayout) {
    setupInstructionsLabel();
    
    QVBoxLayout* pitchLayout = new QVBoxLayout();
    pitchView = new LineupPitchView(this);
    pitchView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pitchView->setMinimumHeight(500); 
    
    pitchLayout->addWidget(instructionsLabel);
    pitchLayout->addWidget(pitchView);

    contentLayout->addLayout(playersListLayout, 2);
    contentLayout->addLayout(pitchLayout, 5);
}

void LineupView::setupInstructionsLabel() {
    instructionsLabel = new QLabel(this);
    instructionsLabel->setText("Drag players to positions on the field or bench. Click on players to view their stats.");
    instructionsLabel->setStyleSheet("color: #666; font-style: italic;");
    instructionsLabel->setAlignment(Qt::AlignCenter);
    instructionsLabel->setWordWrap(true);
    instructionsLabel->setMaximumHeight(40);
}

void LineupView::processDropEvent(QDropEvent* dropEvent, QObject* watched) {
    if (!dropEvent->mimeData()->hasText()) {
        return;
    }
    
    QString mimeText = dropEvent->mimeData()->text();
    if (!mimeText.contains("|")) {
        return;
    }
    
    processDropEventData(dropEvent, watched, mimeText);
}

void LineupView::processDropEventData(QDropEvent* dropEvent, QObject* watched, const QString& mimeText) {
    QStringList parts = mimeText.split("|");
    int playerId = parts[0].toInt();
    QString fromPosition = parts[1];
    QString toPosition = (watched == benchList->viewport()) ? "BENCH" : "RESERVE";
    
    if (fromPosition == toPosition) {
        return;
    }
    
    handleDropPositions(dropEvent, playerId, fromPosition, toPosition);
}

void LineupView::handleDropPositions(QDropEvent* dropEvent, int playerId, const QString& fromPosition, const QString& toPosition) {
    if (fromPosition != "BENCH" && fromPosition != "RESERVE") {
        handleFieldToListDropInEvent(playerId, fromPosition, toPosition);
        dropEvent->acceptProposedAction();
    }
    else if (fromPosition != toPosition) {
        handleListToListDropInEvent(playerId, fromPosition, toPosition);
        dropEvent->acceptProposedAction();
    }
    
    QTimer::singleShot(100, this, &LineupView::updatePlayerLists);
}

void LineupView::handleFieldToListDropInEvent(int playerId, const QString& fromPosition, const QString& toPosition) {
    Player* player = findPlayerById(playerId);
    if (!player) {
        return;
    }
    
    pitchView->clearPosition(fromPosition);
    
    QListWidget* targetList = (toPosition == "BENCH") ? benchList : reservesList;
    QListWidgetItem* item = createPlayerItem(*player);
    targetList->addItem(item);
}

void LineupView::handleListToListDropInEvent(int playerId, const QString& fromPosition, const QString& toPosition) {
    movePlayerBetweenLists(playerId, fromPosition, toPosition);
}

bool LineupView::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Drop) {
        QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
        
        if (watched == benchList->viewport() || watched == reservesList->viewport()) {
            processDropEvent(dropEvent, watched);
            return true;
        }
    }
    
    return QWidget::eventFilter(watched, event);
}

void LineupView::setupConnections() {
    connect(existingLineupsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LineupView::loadSelectedLineup);
            
    connect(newLineupButton, &QPushButton::clicked, 
            this, &LineupView::createNewLineup);
            
    connect(deleteLineupButton, &QPushButton::clicked, 
            this, &LineupView::handleDeleteLineupAction);
    
    setupAdditionalConnections();
}

void LineupView::handleDeleteLineupAction() {
    if (currentLineup.lineupId <= 0 || !currentTeam) {
        return;
    }
    
    if (confirmLineupDeletion()) {
        lineupRatingLabel->setText("--");
        lineupRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white; text-align: right;");
        teamManager.deleteLineup(currentLineup.lineupId);
        loadLineups();
        updateLineupVisibility(false);
    }
}

bool LineupView::confirmLineupDeletion() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Delete Lineup", "Are you sure you want to delete this lineup?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    return reply == QMessageBox::Yes;
}

void LineupView::setupAdditionalConnections() {
    setupPitchViewConnections();
    setupListWidgetConnections();
    
    connectListWidgetItems(benchList);
    connectListWidgetItems(reservesList);
    
    connectDraggableListWidgets();
}

void LineupView::setupPitchViewConnections() {
    connect(pitchView, &LineupPitchView::playerDragDropped,
            this, &LineupView::handleDragDropPlayer);
            
    connect(pitchView, &LineupPitchView::playerClicked,
            this, &LineupView::playerClicked);
}

void LineupView::setupListWidgetConnections() {
    connect(benchList, &QListWidget::itemChanged, [this]() {
        updatePlayerLists();
    });
    
    connect(reservesList, &QListWidget::itemChanged, [this]() {
        updatePlayerLists();
    });
}

void LineupView::connectListWidgetItems(QListWidget* listWidget) {
    connect(listWidget, &QListWidget::itemClicked, [this, listWidget](QListWidgetItem* item) {
        if (item) {
            int playerId = item->data(Qt::UserRole).toInt();
            listWidget->clearSelection();
            emit playerClicked(playerId);
        }
    });
}

void LineupView::connectDraggableListWidgets() {
    auto connectDraggableList = [this](DraggableListWidget* listWidget) {
        connect(listWidget, &DraggableListWidget::fieldPlayerDropped,
            [this](int playerId, const QString& fromPosition, const QString& toListType) {
                handleDragDropPlayer(playerId, fromPosition, toListType);
            });
    };
    
    connectDraggableList(dynamic_cast<DraggableListWidget*>(benchList));
    connectDraggableList(dynamic_cast<DraggableListWidget*>(reservesList));
}

void LineupView::loadLineups() {
    if (!currentTeam) {
        return;
    }
    
    existingLineupsComboBox->clear();
    existingLineupsComboBox->addItem("-- Select Lineup --", -1);
    
    std::vector<Lineup> lineups = teamManager.getTeamLineups(currentTeam->teamId);
    
    populateLineupComboBox(lineups);
}

void LineupView::populateLineupComboBox(const std::vector<Lineup>& lineups) {
    for (const auto& lineup : lineups) {
        addLineupToComboBox(lineup);
    }
}

void LineupView::addLineupToComboBox(const Lineup& lineup) {
    QString formationName = QString::fromStdString(lineup.formationName);
    QString lineupName = QString::fromStdString(lineup.name);
    
    QString displayName = createLineupDisplayName(lineup.lineupId, lineupName, formationName);
    
    existingLineupsComboBox->addItem(displayName, lineup.lineupId);
    
    if (lineup.isActive) {
        highlightActiveLineup();
    }
}

QString LineupView::createLineupDisplayName(int lineupId, const QString& lineupName, const QString& formationName) {
    if (!lineupName.isEmpty()) {
        return lineupName + " (" + formationName + ")";
    } else {
        return QString("Lineup %1 (%2)").arg(lineupId).arg(formationName);
    }
}

void LineupView::highlightActiveLineup() {
    int idx = existingLineupsComboBox->count() - 1;
    QFont font = existingLineupsComboBox->itemData(idx, Qt::FontRole).value<QFont>();
    font.setBold(true);
    existingLineupsComboBox->setItemData(idx, font, Qt::FontRole);
    existingLineupsComboBox->setCurrentIndex(idx);
}

void LineupView::setTeam(Team* team) {
    currentTeam = team;
    currentLineup = Lineup();
    
    resetTeamView();
    
    if (currentTeam) {
        loadTeamData();
    } else {
        disableTeamControls();
    }
    
    updateLineupRatingDisplay();
}

void LineupView::resetTeamView() {
    playerImageCache.clear();
    pitchView->clearPositions();
}

void LineupView::loadTeamData() {
    loadTeamPlayerImages();
    loadLineups();
    loadActiveTeamLineup();
}

void LineupView::loadActiveTeamLineup() {
    Lineup activeLineup = teamManager.getActiveLineup(currentTeam->teamId);
    if (activeLineup.lineupId > 0) {
        setActiveLineup(activeLineup);
    } else {
        enableLineupCreation();
    }
}

void LineupView::setActiveLineup(const Lineup& activeLineup) {
    currentLineup = activeLineup;
    pitchView->setFormation(QString::fromStdString(currentLineup.formationName));
    
    std::set<int> teamPlayerIds = getTeamPlayerIds();
    
    populateFieldPositions(teamPlayerIds);
    populatePlayerLists();
    updateLineupVisibility(true);
}

std::set<int> LineupView::getTeamPlayerIds() {
    std::set<int> teamPlayerIds;
    for (const auto& player : currentTeam->players) {
        teamPlayerIds.insert(player.playerId);
    }
    return teamPlayerIds;
}

void LineupView::enableLineupCreation() {
    updateLineupVisibility(false);
    existingLineupsComboBox->setEnabled(true);
    newLineupButton->setEnabled(true);
}

void LineupView::disableTeamControls() {
    updateLineupVisibility(false);
    existingLineupsComboBox->setEnabled(false);
    newLineupButton->setEnabled(false);
}

void LineupView::loadTeamPlayerImages() {
    if (!currentTeam) {
        return;
    }
    
    for (const auto& player : currentTeam->players) {
        loadPlayerImageFromUrl(player);
    }
}

void LineupView::loadPlayerImageFromUrl(const Player& player) {
    QString imageUrl = getPlayerImageUrl(&player);
    
    if (!imageUrl.isEmpty() && !imageUrl.startsWith("http")) {
        QPixmap playerImage;
        if (playerImage.load(imageUrl)) {
            QPixmap scaledImage = playerImage.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            playerImageCache[player.playerId] = scaledImage;
        }
    }
}

QString LineupView::getPlayerImageUrl(const Player* player) {
    if (!player) {
        return QString();
    }
    
    QString imageUrl = QString::fromStdString(player->imageUrl);
    if (imageUrl.contains(",")) {
        imageUrl = imageUrl.split(",").first().trimmed();
    }
    
    return imageUrl;
}

void LineupView::loadActiveLineup() {
    if (!currentTeam) {
        updateLineupVisibility(false);
        return;
    }
    
    Lineup activeLineup = teamManager.getActiveLineup(currentTeam->teamId);
    processActiveLineup(activeLineup);
}

void LineupView::processActiveLineup(const Lineup& activeLineup) {
    if (activeLineup.lineupId > 0) {
        currentLineup = activeLineup;
        pitchView->setFormation(QString::fromStdString(currentLineup.formationName));
        
        std::set<int> teamPlayerIds = getTeamPlayerIds();
        
        populateFieldPositions(teamPlayerIds);
        populatePlayerLists();
        updateLineupVisibility(true);
    } else {
        updateLineupVisibility(false);
    }
}

bool LineupView::isPlayerInTeam(int playerId, const std::set<int>& teamPlayerIds) {
    return teamPlayerIds.find(playerId) != teamPlayerIds.end();
}

void LineupView::populateFieldPositions(const std::set<int>& teamPlayerIds) {
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::STARTING) {
            if (!isPlayerInTeam(playerPos.playerId, teamPlayerIds)) {
                continue;
            }
            
            setPlayerInPitchView(playerPos.playerId, QString::fromStdString(playerPos.fieldPosition));
        }
    }
}

void LineupView::setPlayerInPitchView(int playerId, const QString& position) {
    Player* player = findPlayerById(playerId);
    if (!player) return;
    
    QPixmap playerImage;
    if (playerImageCache.contains(playerId)) {
        playerImage = playerImageCache[playerId];
    } else {
        QString imageUrl = getPlayerImageUrl(player);
        
        if (!imageUrl.isEmpty()) {
            if (imageUrl.startsWith("http")) {
                loadPlayerImage(playerId, imageUrl, position);
                return;
            }
        }
    }
    
    pitchView->setPlayerAtPosition(
        playerId,
        QString::fromStdString(player->name),
        playerImage,
        position
    );
}

void LineupView::createNewLineup() {
    if (!currentTeam) {
        QMessageBox::warning(this, "Error", "No team selected");
        return;
    }
    
    LineupCreationDialog dialog(teamManager, this);
    if (dialog.exec() == QDialog::Accepted) {
        int formationId = dialog.getSelectedFormationId();
        QString lineupName = dialog.getLineupName();
        
        if (formationId <= 0) {
            QMessageBox::warning(this, "Error", "Please select a formation");
            return;
        }
        
        createAndSetupNewLineup(formationId, lineupName);
    }
}

void LineupView::createAndSetupNewLineup(int formationId, const QString& lineupName) {
    Lineup newLineup = teamManager.createLineup(currentTeam->teamId, formationId, lineupName.toStdString());
    currentLineup = newLineup;
    
    double initialRating = calculateInitialLineupRating();
    
    std::pair<int, int> ratingKey = {currentTeam->teamId, currentLineup.lineupId};
    initialLineupRatings[ratingKey] = initialRating;
    
    pitchView->setFormation(QString::fromStdString(currentLineup.formationName));
    populatePlayerLists();
    loadLineups();
    
    selectNewLineupInComboBox(newLineup.lineupId);
    updateLineupVisibility(true);
}

double LineupView::calculateInitialLineupRating() {
    double totalRating = 0.0;
    int playerCount = 0;
    
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::STARTING) {
            Player* player = findPlayerById(playerPos.playerId);
            if (player) {
                totalRating += player->rating;
                playerCount++;
            }
        }
    }
    
    return playerCount > 0 ? (totalRating / playerCount) : 0.0;
}

void LineupView::selectNewLineupInComboBox(int lineupId) {
    for (int i = 0; i < existingLineupsComboBox->count(); ++i) {
        if (existingLineupsComboBox->itemData(i).toInt() == lineupId) {
            existingLineupsComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void LineupView::loadSelectedLineup(int index) {
    if (index <= 0) {
        updateLineupVisibility(false);
        return;
    }
    
    int lineupId = existingLineupsComboBox->itemData(index).toInt();
    
    if (lineupId <= 0) {
        updateLineupVisibility(false);
        return;
    }
    
    loadLineupById(lineupId);
}

void LineupView::loadLineupById(int lineupId) {
    std::vector<Lineup> lineups = teamManager.getTeamLineups(currentTeam->teamId);
    for (const auto& lineup : lineups) {
        if (lineup.lineupId == lineupId) {
            clearAndReloadLineup(lineup);
            return;
        }
    }
}

void LineupView::clearAndReloadLineup(const Lineup& lineup) {
    currentLineup = lineup;
    pitchView->setFormation(QString::fromStdString(currentLineup.formationName));
    
    loadStartingPlayers();
    
    populatePlayerLists();
    updateLineupVisibility(true);
    updateLineupRatingDisplay();
}

void LineupView::loadStartingPlayers() {
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::STARTING) {
            setPlayerInPitchView(playerPos.playerId, QString::fromStdString(playerPos.fieldPosition));
        }
    }
}

void LineupView::movePlayerBetweenLists(int playerId, const QString& fromPosition, const QString& toPosition) {
    QListWidget* sourceList = getListWidgetByPosition(fromPosition);
    QListWidget* destList = getListWidgetByPosition(toPosition);
    
    Player* player = findPlayerById(playerId);
    if (player) {
        QListWidgetItem* newItem = createPlayerItem(*player);
        destList->addItem(newItem);
        
        removePlayerFromSourceList(playerId, sourceList);
    }
}

QListWidget* LineupView::getListWidgetByPosition(const QString& position) {
    return (position == "RESERVE") ? reservesList : benchList;
}

void LineupView::removePlayerFromSourceList(int playerId, QListWidget* sourceList) {
    for (int i = 0; i < sourceList->count(); ++i) {
        QListWidgetItem* item = sourceList->item(i);
        if (item->data(Qt::UserRole).toInt() == playerId) {
            delete sourceList->takeItem(i);
            break;
        }
    }
}

void LineupView::saveCurrentLineup(bool forceSave) {
    if (currentLineup.lineupId <= 0 || !currentTeam) {
        QMessageBox::warning(this, "Error", "No lineup selected");
        return;
    }
    
    collectAllPlayerPositions();
    
    currentLineup.isActive = true;
    teamManager.saveLineup(currentLineup);
    loadLineups();
    updateLineupRatingDisplay();
}

void LineupView::collectAllPlayerPositions() {
    currentLineup.playerPositions.clear();
    collectPlayerPositionsFromPitch();
    collectPlayerPositionsFromBench();
    collectPlayerPositionsFromReserves();
    addRemainingPlayersToReserves();
}

void LineupView::collectPlayerPositionsFromPitch() {
    QMap<QString, int> pitchPlayers = pitchView->getPlayersPositions();
    QSet<int> processedPlayers;
    
    for (auto it = pitchPlayers.begin(); it != pitchPlayers.end(); ++it) {
        if (it.value() > 0) {
            addPlayerToPositions(it.value(), PositionType::STARTING, it.key(), 0);
            processedPlayers.insert(it.value());
        }
    }
}

void LineupView::addPlayerToPositions(int playerId, PositionType posType, const QString& fieldPos, int order) {
    PlayerPosition pos;
    pos.playerId = playerId;
    pos.positionType = posType;
    pos.fieldPosition = fieldPos.toStdString();
    pos.order = order;
    currentLineup.playerPositions.push_back(pos);
}

void LineupView::collectPlayerPositionsFromBench() {
    collectPlayersFromList(benchList, PositionType::BENCH);
}

void LineupView::collectPlayerPositionsFromReserves() {
    collectPlayersFromList(reservesList, PositionType::RESERVE);
}

void LineupView::collectPlayersFromList(QListWidget* listWidget, PositionType posType) {
    QSet<int> processedPlayers;
    
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* item = listWidget->item(i);
        int playerId = item->data(Qt::UserRole).toInt();
        
        if (!processedPlayers.contains(playerId)) {
            addPlayerToPositions(playerId, posType, "", i);
            processedPlayers.insert(playerId);
        }
    }
}

void LineupView::addRemainingPlayersToReserves() {
    if (!currentTeam) {
        return;
    }
    
    QSet<int> processedPlayers = getProcessedPlayerIds();
    
    for (const auto& player : currentTeam->players) {
        if (!processedPlayers.contains(player.playerId)) {
            addPlayerToPositions(player.playerId, PositionType::RESERVE, "", 0);
        }
    }
}

QSet<int> LineupView::getProcessedPlayerIds() {
    QSet<int> processedPlayers;
    for (const auto& pos : currentLineup.playerPositions) {
        processedPlayers.insert(pos.playerId);
    }
    return processedPlayers;
}

void LineupView::handleDragDropPlayer(int playerId, const QString& fromPosition, const QString& toPosition) {
    if (!currentTeam) return;
    
    Player* player = findPlayerById(playerId);
    if (!player) {
        return;
    }
    
    processPlayerDragDrop(playerId, fromPosition, toPosition);
    
    saveCurrentLineup(true);
    updateLineupRatingDisplay();
}

void LineupView::processPlayerDragDrop(int playerId, const QString& fromPosition, const QString& toPosition) {
    bool isFromField = (fromPosition != "BENCH" && fromPosition != "RESERVE");
    bool isToList = (toPosition == "BENCH" || toPosition == "RESERVE");
    
    if (isFromField && isToList) {
        handleFieldToListDrop(playerId, fromPosition, toPosition);
        return;
    }
    
    int existingPlayerId = findExistingPlayerAtPosition(toPosition);
    
    if (toPosition == "BENCH" || toPosition == "RESERVE") {
        handleListToListDrop(playerId, fromPosition, toPosition);
    }
    else {
        handleListToFieldDrop(playerId, fromPosition, toPosition, existingPlayerId);
    }
}

int LineupView::findExistingPlayerAtPosition(const QString& position) {
    int existingPlayerId = -1;
    QMap<QString, int> currentPositions = pitchView->getPlayersPositions();
    if (currentPositions.contains(position)) {
        existingPlayerId = currentPositions[position];
    }
    return existingPlayerId;
}

void LineupView::handleFieldToListDrop(int playerId, const QString& fromPosition, const QString& toPosition) {
    pitchView->clearPosition(fromPosition);
    
    QListWidget* targetList = (toPosition == "BENCH") ? benchList : reservesList;
    
    if (!isPlayerInListAlready(playerId, targetList)) {
        addPlayerToList(playerId, targetList);
    }
    
    scheduleSave();
}

void LineupView::scheduleSave() {
    QTimer::singleShot(100, this, [this]() {
        saveCurrentLineup(true);
    });
}

void LineupView::addPlayerToList(int playerId, QListWidget* targetList) {
    Player* player = findPlayerById(playerId);
    if (player) {
        QListWidgetItem* item = createPlayerItem(*player);
        targetList->addItem(item);
    }
}

bool LineupView::isPlayerInListAlready(int playerId, QListWidget* targetList) {
    if (!targetList) {
        return false;
    }
    
    for (int i = 0; i < targetList->count(); i++) {
        if (targetList->item(i)->data(Qt::UserRole).toInt() == playerId) {
            return true;
        }
    }
    return false;
}

void LineupView::handleListToListDrop(int playerId, const QString& fromPosition, const QString& toPosition) {
    if (fromPosition == toPosition) {
        return;
    }
    
    QListWidget* targetList = (toPosition == "BENCH") ? benchList : reservesList;
    
    if (isFromField(fromPosition)) {
        movePlayerFromFieldToList(playerId, fromPosition, targetList);
    }
    else {
        movePlayerBetweenLists(playerId, fromPosition, toPosition);
    }
}

bool LineupView::isFromField(const QString& position) {
    return position != "BENCH" && position != "RESERVE";
}

void LineupView::movePlayerFromFieldToList(int playerId, const QString& fromPosition, QListWidget* targetList) {
    pitchView->clearPosition(fromPosition);
    addPlayerToList(playerId, targetList);
}

void LineupView::removePlayerFromList(int playerId, QListWidget* sourceList) {
    if (!sourceList) {
        return;
    }
    
    for (int i = 0; i < sourceList->count(); ++i) {
        QListWidgetItem* item = sourceList->item(i);
        if (item->data(Qt::UserRole).toInt() == playerId) {
            delete sourceList->takeItem(i);
            break;
        }
    }
}

void LineupView::handleListToFieldDrop(int playerId, const QString& fromPosition, const QString& toPosition, int existingPlayerId) {
    if (existingPlayerId > 0 && existingPlayerId != playerId) {
        handleExistingPlayerMove(existingPlayerId, fromPosition);
    }
    
    Player* player = findPlayerById(playerId);
    if (!player) return;
    
    QPixmap playerImage = getPlayerImage(playerId, toPosition);
    
    pitchView->setPlayerAtPosition(
        playerId,
        QString::fromStdString(player->name),
        playerImage,
        toPosition
    );
    
    if (isListPosition(fromPosition)) {
        removePlayerFromListPosition(playerId, fromPosition);
    }
}

bool LineupView::isListPosition(const QString& position) {
    return position == "RESERVE" || position == "BENCH";
}

void LineupView::removePlayerFromListPosition(int playerId, const QString& position) {
    QListWidget* sourceList = (position == "RESERVE") ? reservesList : benchList;
    removePlayerFromList(playerId, sourceList);
}

QPixmap LineupView::getPlayerImage(int playerId, const QString& position) {
    if (playerImageCache.contains(playerId)) {
        return playerImageCache[playerId];
    }
    
    return loadPlayerImageFromSource(playerId, position);
}

QPixmap LineupView::loadPlayerImageFromSource(int playerId, const QString& position) {
    QPixmap playerImage;
    Player* player = findPlayerById(playerId);
    if (!player) return playerImage;
    
    QString imageUrl = getPlayerImageUrl(player);
    
    if (!imageUrl.isEmpty()) {
        if (imageUrl.startsWith("http")) {
            loadPlayerImage(playerId, imageUrl, position);
        } else if (QFile::exists(imageUrl)) {
            playerImage.load(imageUrl);
            playerImageCache[playerId] = playerImage;
        }
    }
    
    return playerImage;
}

void LineupView::handleExistingPlayerMove(int existingPlayerId, const QString& fromPosition) {
    Player* existingPlayer = findPlayerById(existingPlayerId);
    if (!existingPlayer) {
        return;
    }
    
    if (!isListPosition(fromPosition)) {
        moveExistingPlayerToField(existingPlayerId, existingPlayer, fromPosition);
    } 
    else {
        moveExistingPlayerToList(existingPlayer, fromPosition);
    }
}

void LineupView::moveExistingPlayerToField(int existingPlayerId, Player* existingPlayer, const QString& position) {
    QPixmap existingPlayerImage = getExistingPlayerImage(existingPlayerId);
    
    pitchView->setPlayerAtPosition(
        existingPlayerId, 
        QString::fromStdString(existingPlayer->name),
        existingPlayerImage,
        position
    );
}

QPixmap LineupView::getExistingPlayerImage(int playerId) {
    QPixmap existingPlayerImage;
    if (playerImageCache.contains(playerId)) {
        existingPlayerImage = playerImageCache[playerId];
    }
    return existingPlayerImage;
}

void LineupView::moveExistingPlayerToList(Player* player, const QString& targetPosition) {
    QListWidget* targetList = (targetPosition == "RESERVE") ? reservesList : benchList;
    QListWidgetItem* item = createPlayerItem(*player);
    targetList->addItem(item);
}

void LineupView::updatePlayerLists() {
    saveCurrentLineup(true);
}

void LineupView::populatePlayerLists() {
    if (!currentTeam || currentLineup.lineupId <= 0) {
        return;
    }

    clearPlayerLists();
    
    std::set<int> teamPlayerIds = getTeamPlayerIds();
    
    QSet<int> usedPlayerIds;
    collectStartingPlayers(teamPlayerIds, usedPlayerIds);
    collectBenchPlayers(teamPlayerIds, usedPlayerIds);
    collectReservePlayers(teamPlayerIds, usedPlayerIds);
    
    addRemainingPlayersToReserveList(usedPlayerIds);
}

void LineupView::clearPlayerLists() {
    benchList->clear();
    reservesList->clear();
}

void LineupView::collectStartingPlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds) {
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::STARTING) {
            if (isPlayerInTeam(playerPos.playerId, teamPlayerIds)) {
                usedPlayerIds.insert(playerPos.playerId);
            }
        }
    }
}

void LineupView::collectBenchPlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds) {
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType != PositionType::BENCH) {
            continue;
        }
        
        if (!isPlayerInTeam(playerPos.playerId, teamPlayerIds)) {
            continue;
        }
        
        addPlayerToListAndTrack(playerPos.playerId, benchList, usedPlayerIds);
    }
}

void LineupView::addPlayerToListAndTrack(int playerId, QListWidget* list, QSet<int>& usedPlayerIds) {
    Player* player = findPlayerById(playerId);
    if (player) {
        QListWidgetItem* item = createPlayerItem(*player);
        list->addItem(item);
        usedPlayerIds.insert(playerId);
    }
}

void LineupView::collectReservePlayers(const std::set<int>& teamPlayerIds, QSet<int>& usedPlayerIds) {
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType != PositionType::RESERVE) {
            continue;
        }
        
        if (!isPlayerInTeam(playerPos.playerId, teamPlayerIds)) {
            continue;
        }
        
        addPlayerToListAndTrack(playerPos.playerId, reservesList, usedPlayerIds);
    }
}

void LineupView::addRemainingPlayersToReserveList(const QSet<int>& usedPlayerIds) {
    if (!currentTeam) {
        return;
    }
    
    for (const auto& player : currentTeam->players) {
        if (!usedPlayerIds.contains(player.playerId)) {
            QListWidgetItem* item = createPlayerItem(player);
            reservesList->addItem(item);
        }
    }
}

void LineupView::updateLineupVisibility(bool visible) {
    lineupStackedWidget->setCurrentIndex(visible ? 1 : 0);
    deleteLineupButton->setEnabled(visible);
    instructionsLabel->setVisible(visible);
}

QListWidgetItem* LineupView::createPlayerItem(const Player& player) {
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(QString::fromStdString(player.name));
    item->setData(Qt::UserRole, player.playerId);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    
    item->setToolTip(createPlayerTooltip(player));
    
    item->setSelected(false);
    
    return item;
}

QString LineupView::createPlayerTooltip(const Player& player) {
    return QString("%1 - %2")
                 .arg(QString::fromStdString(player.position))
                 .arg(QString::fromStdString(player.subPosition));
}

Player* LineupView::findPlayerById(int playerId) {
    if (!currentTeam) return nullptr;
    
    for (auto& player : currentTeam->players) {
        if (player.playerId == playerId) {
            return &player;
        }
    }
    
    return nullptr;
}

void LineupView::loadPlayerImage(int playerId, const QString& imageUrl, const QString& position) {
    if (imageUrl.isEmpty()) return;
    
    QNetworkAccessManager* networkManager = new QNetworkAccessManager(this);
    QUrl url(imageUrl);
    QNetworkRequest request(url);
    QNetworkReply* reply = networkManager->get(request);
    
    connectImageLoadingReply(reply, playerId, position);
}

void LineupView::connectImageLoadingReply(QNetworkReply* reply, int playerId, const QString& position) {
    connect(reply, &QNetworkReply::finished, this, [this, reply, playerId, position]() {
        this->handleImageLoaded(reply, playerId, position);
    });
}

void LineupView::handleImageLoaded(QNetworkReply* reply, int playerId, const QString& position) {
    if (reply->error() == QNetworkReply::NoError) {
        processLoadedImage(reply, playerId, position);
    }
    
    cleanupReply(reply);
}

void LineupView::processLoadedImage(QNetworkReply* reply, int playerId, const QString& position) {
    QPixmap pixmap;
    pixmap.loadFromData(reply->readAll());
    
    playerImageCache[playerId] = pixmap;
    
    Player* player = findPlayerById(playerId);
    if (player) {
        pitchView->setPlayerAtPosition(
            playerId, 
            QString::fromStdString(player->name),
            pixmap,
            position
        );
    }
}

void LineupView::cleanupReply(QNetworkReply* reply) {
    reply->deleteLater();
    sender()->deleteLater();
}

void LineupView::setupLineupRatingDisplay() {
    lineupRatingWidget = new QWidget(this);
    lineupRatingWidget->setFixedWidth(200);
    
    QVBoxLayout* ratingLayout = new QVBoxLayout(lineupRatingWidget);
    setupRatingLayout(ratingLayout);
    
    hasInitialLineupRating = false;
    lineupRatingWidget->setVisible(true);
}

void LineupView::setupRatingLayout(QVBoxLayout* ratingLayout) {
    ratingLayout->setContentsMargins(0, 0, 0, 0);
    ratingLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    ratingLayout->setSpacing(2);
    
    QLabel* lineupRatingTitle = new QLabel("Lineup Rating", lineupRatingWidget);
    lineupRatingTitle->setStyleSheet("font-size: 12px; color: #aaaaaa;");
    lineupRatingTitle->setAlignment(Qt::AlignRight);
    
    lineupRatingLabel = new QLabel("--", lineupRatingWidget);
    lineupRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white;");
    lineupRatingLabel->setAlignment(Qt::AlignRight);
    
    ratingLayout->addWidget(lineupRatingTitle);
    ratingLayout->addWidget(lineupRatingLabel);
}

void LineupView::updateLineupRatingDisplay() {
    if (!currentTeam || currentLineup.lineupId <= 0) {
        resetRatingDisplay();
        return;
    }
    
    double averageRating = calculateCurrentLineupRating();
    
    if (averageRating <= 0) {
        resetRatingDisplay();
        return;
    }
    
    std::pair<int, int> ratingKey = {currentTeam->teamId, currentLineup.lineupId};
    setupInitialRatingIfNeeded(ratingKey, averageRating);
    
    double ratingDiff = averageRating - initialLineupRatings[ratingKey];
    updateRatingLabel(averageRating, ratingDiff);
}

void LineupView::resetRatingDisplay() {
    if (lineupRatingLabel) {
        lineupRatingLabel->setText("--");
        lineupRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white; text-align: right;");
    }
}

double LineupView::calculateCurrentLineupRating() {
    double totalRating = 0.0;
    int playerCount = 0;
    
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::STARTING) {
            Player* player = findPlayerById(playerPos.playerId);
            if (player) {
                totalRating += player->rating;
                playerCount++;
            }
        }
    }
    
    return playerCount > 0 ? (totalRating / playerCount) : 0.0;
}

void LineupView::setupInitialRatingIfNeeded(const std::pair<int, int>& ratingKey, double averageRating) {
    if (initialLineupRatings.find(ratingKey) == initialLineupRatings.end()) {
        initialLineupRatings[ratingKey] = averageRating;
    }
}

void LineupView::updateRatingLabel(double averageRating, double ratingDiff) {
    QString displayText;
    QString styleSheet;
    
    if (ratingDiff > 0.01) {
        displayText = formatPositiveRatingChange(averageRating, ratingDiff);
        styleSheet = "font-size: 14px; font-weight: bold; color: #4CAF50; text-align: right;";
    } else if (ratingDiff < -0.01) {
        displayText = formatNegativeRatingChange(averageRating, ratingDiff);
        styleSheet = "font-size: 14px; font-weight: bold; color: #F44336; text-align: right;";
    } else {
        displayText = formatUnchangedRating(averageRating);
        styleSheet = "font-size: 14px; font-weight: bold; color: white; text-align: right;";
    }
    
    lineupRatingLabel->setText(displayText);
    lineupRatingLabel->setStyleSheet(styleSheet);
}

QString LineupView::formatPositiveRatingChange(double averageRating, double ratingDiff) {
    return QString("%1 <span style='font-size: 12px; color: #4CAF50;'>+%2</span>")
                .arg(averageRating, 0, 'f', 1)
                .arg(ratingDiff, 0, 'f', 1);
}

QString LineupView::formatNegativeRatingChange(double averageRating, double ratingDiff) {
    return QString("%1 <span style='font-size: 12px; color: #F44336;'>%2</span>")
                .arg(averageRating, 0, 'f', 1)
                .arg(ratingDiff, 0, 'f', 1);
}

QString LineupView::formatUnchangedRating(double averageRating) {
    return QString("%1").arg(averageRating, 0, 'f', 1);
}

#include "gui/views/LineupView.h"
#include "gui/components/widgets/DraggableListWidget.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGroupBox>
#include <QTimer>
#include <QFile>
#include <algorithm>
#include <ranges>

LineupView::LineupView(TeamManager& teamManager, Team* currentTeam, QWidget *parent)
    : QWidget(parent)
    , m_teamManager(teamManager)
    , m_currentTeam(currentTeam)
    , m_networkManager(std::make_unique<QNetworkAccessManager>(this))
{
    setupLineupRatingDisplay();
    setupUi();
    setupConnections();
    
    if (m_currentTeam) {
        setTeam(m_currentTeam);
    }
}

LineupView::~LineupView() {
    disconnect(this, nullptr, nullptr, nullptr);
    
    if (m_benchList) {
        disconnect(m_benchList, nullptr, nullptr, nullptr);
    }
    
    if (m_reservesList) {
        disconnect(m_reservesList, nullptr, nullptr, nullptr);
    }
    
    if (m_pitchView) {
        disconnect(m_pitchView, nullptr, nullptr, nullptr);
    }
}

void LineupView::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(10);
    
    auto* viewButtonsLayout = new QHBoxLayout();
    viewButtonsLayout->setContentsMargins(10, 10, 10, 0);
    mainLayout->addLayout(viewButtonsLayout);

    setupControlsLayout(mainLayout);
    setupLineupContent();
    
    mainLayout->addWidget(m_lineupScrollArea, 1);
    
    updateLineupVisibility(false);

    m_benchList->viewport()->installEventFilter(this);
    m_reservesList->viewport()->installEventFilter(this);
}

QWidget* LineupView::createPlaceholderWidget(QWidget* parent) {
    auto* placeholderWidget = new QWidget(parent);
    auto* placeholderLayout = new QVBoxLayout(placeholderWidget);
    
    setupPlaceholderLabels(placeholderLayout, parent);
    
    placeholderLayout->addStretch(1);
    placeholderLayout->setSpacing(10);
    
    return placeholderWidget;
}

void LineupView::setupPlaceholderLabels(QVBoxLayout* layout, QWidget* parent) {
    auto* noLineupLabel = new QLabel("No lineup selected", parent);
    noLineupLabel->setAlignment(Qt::AlignCenter);
    noLineupLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #666;");
    
    auto* instructionLabel = new QLabel("Create a new lineup or select an existing one", parent);
    instructionLabel->setAlignment(Qt::AlignCenter);
    instructionLabel->setStyleSheet("font-size: 14px; color: #888;");
    
    auto* line = new QFrame(parent);
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
    
    if (m_pitchView) {
        int containerWidth = m_lineupScrollArea->width() - 40;
        m_pitchView->setMinimumWidth(containerWidth * 0.65);
    }
    
    if (m_benchList && m_reservesList) {
        int listWidth = m_lineupScrollArea->width() * 0.25;
        m_benchList->setMinimumWidth(listWidth);
        m_reservesList->setMinimumWidth(listWidth);
    }
}

void LineupView::setupControlsLayout(QVBoxLayout* mainLayout) {
    auto* controlsLayout = new QGridLayout();
    controlsLayout->setContentsMargins(10, 0, 10, 10);
    controlsLayout->setHorizontalSpacing(15);
    controlsLayout->setVerticalSpacing(10);
    
    setupLineupControls(controlsLayout);
    
    auto* topLayout = new QHBoxLayout();
    topLayout->addLayout(controlsLayout);
    topLayout->addStretch(1);
    topLayout->addWidget(m_lineupRatingWidget, 0, Qt::AlignRight | Qt::AlignTop);
    
    mainLayout->addLayout(topLayout);
}

void LineupView::setupLineupControls(QGridLayout* controlsLayout) {
    auto* existingLineupsLabel = new QLabel("Lineups:", this);
    m_existingLineupsComboBox = new QComboBox(this);
    m_existingLineupsComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_existingLineupsComboBox->setMinimumWidth(150);
    
    m_newLineupButton = new QPushButton("Create Lineup", this);
    m_newLineupButton->setEnabled(m_currentTeam != nullptr);
    m_deleteLineupButton = new QPushButton("Delete Lineup", this);
    
    auto* dropdownAndButtonsLayout = new QHBoxLayout();
    dropdownAndButtonsLayout->setSpacing(10);
    dropdownAndButtonsLayout->addWidget(m_existingLineupsComboBox, 1);
    dropdownAndButtonsLayout->addSpacing(15);
    dropdownAndButtonsLayout->addWidget(m_newLineupButton);
    dropdownAndButtonsLayout->addWidget(m_deleteLineupButton);
    
    controlsLayout->addWidget(existingLineupsLabel, 0, 0);
    controlsLayout->addLayout(dropdownAndButtonsLayout, 0, 1);
}

void LineupView::setupLineupContent() {
    m_lineupContentWidget = new QWidget(this);
    auto* contentLayout = new QHBoxLayout(m_lineupContentWidget);
    contentLayout->setContentsMargins(10, 0, 10, 10);
    
    auto* playersListLayout = new QVBoxLayout();
    playersListLayout->setSpacing(10);
    
    setupBenchAndReservesGroups(playersListLayout);
    setupPitchLayout(contentLayout, playersListLayout);

    setupStackedWidget();
}

void LineupView::setupStackedWidget() {
    auto* placeholderWidget = createPlaceholderWidget(this);
    
    m_lineupStackedWidget = new QStackedWidget(this);
    m_lineupStackedWidget->addWidget(placeholderWidget);
    m_lineupStackedWidget->addWidget(m_lineupContentWidget);
    
    m_lineupScrollArea = new QScrollArea(this);
    m_lineupScrollArea->setWidgetResizable(true);
    m_lineupScrollArea->setWidget(m_lineupStackedWidget);
    m_lineupScrollArea->setFrameShape(QFrame::NoFrame);
}

void LineupView::setupBenchAndReservesGroups(QVBoxLayout* playersListLayout) {
    auto* benchGroup = createBenchGroup();
    auto* reservesGroup = createReservesGroup();
    
    playersListLayout->addWidget(benchGroup);
    playersListLayout->addWidget(reservesGroup, 1);
}

QGroupBox* LineupView::createBenchGroup() {
    auto* benchGroup = new QGroupBox("Bench", this);
    auto* benchLayout = new QVBoxLayout(benchGroup);
    m_benchList = new DraggableListWidget("BENCH", this);
    setupListWidget(m_benchList);
    m_benchList->setFixedHeight(150);
    benchLayout->addWidget(m_benchList);
    
    return benchGroup;
}

QGroupBox* LineupView::createReservesGroup() {
    auto* reservesGroup = new QGroupBox("Reserves", this);
    auto* reservesLayout = new QVBoxLayout(reservesGroup);
    m_reservesList = new DraggableListWidget("RESERVE", this);
    setupListWidget(m_reservesList);
    reservesLayout->addWidget(m_reservesList);
    
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
    
    auto* pitchLayout = new QVBoxLayout();
    m_pitchView = new LineupPitchView(this);
    m_pitchView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_pitchView->setMinimumHeight(500); 
    
    pitchLayout->addWidget(m_instructionsLabel);
    pitchLayout->addWidget(m_pitchView);

    contentLayout->addLayout(playersListLayout, 2);
    contentLayout->addLayout(pitchLayout, 5);
}

void LineupView::setupInstructionsLabel() {
    m_instructionsLabel = new QLabel(this);
    m_instructionsLabel->setText("Drag players to positions on the field or bench. Click on players to view their stats.");
    m_instructionsLabel->setStyleSheet("color: #666; font-style: italic;");
    m_instructionsLabel->setAlignment(Qt::AlignCenter);
    m_instructionsLabel->setWordWrap(true);
    m_instructionsLabel->setMaximumHeight(40);
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
    QString toPosition = (watched == m_benchList->viewport()) ? "BENCH" : "RESERVE";
    
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
    
    m_pitchView->clearPosition(fromPosition);
    
    QListWidget* targetList = (toPosition == "BENCH") ? m_benchList : m_reservesList;
    QListWidgetItem* item = createPlayerItem(*player);
    targetList->addItem(item);
}

void LineupView::handleListToListDropInEvent(int playerId, const QString& fromPosition, const QString& toPosition) {
    movePlayerBetweenLists(playerId, fromPosition, toPosition);
}

bool LineupView::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Drop) {
        auto* dropEvent = static_cast<QDropEvent*>(event);
        
        if (watched == m_benchList->viewport() || watched == m_reservesList->viewport()) {
            processDropEvent(dropEvent, watched);
            return true;
        }
    }
    
    return QWidget::eventFilter(watched, event);
}

void LineupView::setupConnections() {
    connect(m_existingLineupsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LineupView::loadSelectedLineup);
            
    connect(m_newLineupButton, &QPushButton::clicked, 
            this, &LineupView::createNewLineup);
            
    connect(m_deleteLineupButton, &QPushButton::clicked, 
            this, &LineupView::handleDeleteLineupAction);
    
    setupAdditionalConnections();
}

void LineupView::handleDeleteLineupAction() {
    if (m_currentLineup.lineupId <= 0 || !m_currentTeam) {
        return;
    }
    
    if (confirmLineupDeletion()) {
        m_lineupRatingLabel->setText("--");
        m_lineupRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white; text-align: right;");
        m_teamManager.deleteLineup(m_currentLineup.lineupId);
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
    
    connectListWidgetItems(m_benchList);
    connectListWidgetItems(m_reservesList);
    
    connectDraggableListWidgets();
}

void LineupView::setupPitchViewConnections() {
    connect(m_pitchView, &LineupPitchView::playerDragDropped,
            this, &LineupView::handleDragDropPlayer);
            
    connect(m_pitchView, &LineupPitchView::playerClicked,
            this, &LineupView::playerClicked);
}

void LineupView::setupListWidgetConnections() {
    connect(m_benchList, &QListWidget::itemChanged, [this]() {
        updatePlayerLists();
    });
    
    connect(m_reservesList, &QListWidget::itemChanged, [this]() {
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
    
    connectDraggableList(dynamic_cast<DraggableListWidget*>(m_benchList));
    connectDraggableList(dynamic_cast<DraggableListWidget*>(m_reservesList));
}

void LineupView::loadLineups() {
    if (!m_currentTeam) {
        return;
    }
    
    m_existingLineupsComboBox->clear();
    m_existingLineupsComboBox->addItem("-- Select Lineup --", -1);
    
    std::vector<Lineup> lineups = m_teamManager.getTeamLineups(m_currentTeam->teamId);
    
    for (const auto& lineup : lineups) {
        addLineupToComboBox(lineup);
    }
}

void LineupView::addLineupToComboBox(const Lineup& lineup) {
    QString formationName = QString::fromStdString(lineup.formationName);
    QString lineupName = QString::fromStdString(lineup.name);
    
    QString displayName = createLineupDisplayName(lineup.lineupId, lineupName, formationName);
    
    m_existingLineupsComboBox->addItem(displayName, lineup.lineupId);
    
    if (lineup.isActive) {
        highlightActiveLineup();
    }
}

QString LineupView::createLineupDisplayName(int lineupId, const QString& lineupName, const QString& formationName) const {
    if (!lineupName.isEmpty()) {
        return lineupName + " (" + formationName + ")";
    }
    
    return QString("Lineup %1 (%2)").arg(lineupId).arg(formationName);
}

void LineupView::highlightActiveLineup() {
    int idx = m_existingLineupsComboBox->count() - 1;
    QFont font = m_existingLineupsComboBox->itemData(idx, Qt::FontRole).value<QFont>();
    font.setBold(true);
    m_existingLineupsComboBox->setItemData(idx, font, Qt::FontRole);
    m_existingLineupsComboBox->setCurrentIndex(idx);
}

void LineupView::setTeam(Team* team) {
    m_currentTeam = team;
    m_currentLineup = Lineup();
    
    m_playerImageCache.clear();
    m_pitchView->clearPositions();
    m_benchList->clear();
    m_reservesList->clear();
    
    if (m_currentTeam) {
        loadTeamPlayerImages();
        loadLineups();
        loadActiveTeamLineup();
    } else {
        updateLineupVisibility(false);
        m_existingLineupsComboBox->setEnabled(false);
        m_newLineupButton->setEnabled(false);
    }
    
    updateLineupRatingDisplay();
}

void LineupView::loadActiveTeamLineup() {
    Lineup activeLineup = m_teamManager.getActiveLineup(m_currentTeam->teamId);
    if (activeLineup.lineupId > 0) {
        setActiveLineup(activeLineup);
    } else {
        updateLineupVisibility(false);
        m_existingLineupsComboBox->setEnabled(true);
        m_newLineupButton->setEnabled(true);
    }
}

void LineupView::setActiveLineup(const Lineup& activeLineup) {
    m_currentLineup = activeLineup;
    m_pitchView->setFormation(QString::fromStdString(m_currentLineup.formationName));
    
    std::set<int> teamPlayerIds = getTeamPlayerIds();
    
    populateFieldPositions(teamPlayerIds);
    populatePlayerLists();
    updateLineupVisibility(true);
}

std::set<int> LineupView::getTeamPlayerIds() const {
    if (!m_currentTeam) {
        return {};
    }

    std::set<int> teamPlayerIds;
    for (const auto& player : m_currentTeam->players) {
        teamPlayerIds.insert(player.playerId);
    }

    return teamPlayerIds;
}

void LineupView::loadTeamPlayerImages() {
    if (!m_currentTeam) {
        return;
    }
    
    for (const auto& player : m_currentTeam->players) {
        loadPlayerImageFromUrl(player);
    }
}

void LineupView::loadPlayerImageFromUrl(const Player& player) {
    QString imageUrl = getPlayerImageUrl(&player);
    
    if (!imageUrl.isEmpty() && !imageUrl.startsWith("http")) {
        QPixmap playerImage;
        if (playerImage.load(imageUrl)) {
            QPixmap scaledImage = playerImage.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_playerImageCache[player.playerId] = scaledImage;
        }
    }
}

QString LineupView::getPlayerImageUrl(const Player* player) const {
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
    if (!m_currentTeam) {
        updateLineupVisibility(false);
        return;
    }
    
    Lineup activeLineup = m_teamManager.getActiveLineup(m_currentTeam->teamId);
    
    if (activeLineup.lineupId > 0) {
        m_currentLineup = activeLineup;
        m_pitchView->setFormation(QString::fromStdString(m_currentLineup.formationName));
        
        std::set<int> teamPlayerIds = getTeamPlayerIds();
        
        populateFieldPositions(teamPlayerIds);
        populatePlayerLists();
        updateLineupVisibility(true);
    } else {
        updateLineupVisibility(false);
    }
}

bool LineupView::isPlayerInTeam(int playerId, const std::set<int>& teamPlayerIds) const {
    return teamPlayerIds.find(playerId) != teamPlayerIds.end();
}

void LineupView::populateFieldPositions(const std::set<int>& teamPlayerIds) {
    for (const auto& playerPos : m_currentLineup.playerPositions) {
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
    if (m_playerImageCache.contains(playerId)) {
        playerImage = m_playerImageCache[playerId];
    } else {
        QString imageUrl = getPlayerImageUrl(player);
        
        if (!imageUrl.isEmpty()) {
            if (imageUrl.startsWith("http")) {
                loadPlayerImage(playerId, imageUrl, position);
                return;
            }
        }
    }
    
    m_pitchView->setPlayerAtPosition(
        playerId,
        QString::fromStdString(player->name),
        playerImage,
        position
    );
}

void LineupView::createNewLineup() {
    if (!m_currentTeam) {
        QMessageBox::warning(this, "Error", "No team selected");
        return;
    }
    
    LineupCreationDialog dialog(m_teamManager, this);
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
    Lineup newLineup = m_teamManager.createLineup(m_currentTeam->teamId, formationId, lineupName.toStdString());
    m_currentLineup = newLineup;
    
    double initialRating = calculateInitialLineupRating();
    
    std::pair<int, int> ratingKey = {m_currentTeam->teamId, m_currentLineup.lineupId};
    m_initialLineupRatings[ratingKey] = initialRating;
    
    m_pitchView->setFormation(QString::fromStdString(m_currentLineup.formationName));

    populatePlayerLists();
    loadLineups();
    
    for (int i = 0; i < m_existingLineupsComboBox->count(); ++i) {
        if (m_existingLineupsComboBox->itemData(i).toInt() == newLineup.lineupId) {
            m_existingLineupsComboBox->setCurrentIndex(i);
            break;
        }
    }

    updateLineupVisibility(true);
}

double LineupView::calculateInitialLineupRating() const {
    double totalRating = 0.0;
    int playerCount = 0;
    
    for (const auto& playerPos : m_currentLineup.playerPositions) {
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

void LineupView::loadSelectedLineup(int index) {
    if (index <= 0) {
        updateLineupVisibility(false);
        return;
    }
    
    int lineupId = m_existingLineupsComboBox->itemData(index).toInt();
    
    if (lineupId <= 0) {
        updateLineupVisibility(false);
        return;
    }
    
    std::vector<Lineup> lineups = m_teamManager.getTeamLineups(m_currentTeam->teamId);

    for (const auto& lineup : lineups) {
        if (lineup.lineupId == lineupId) {
            clearAndReloadLineup(lineup);
            return;
        }
    }
}

void LineupView::clearAndReloadLineup(const Lineup& lineup) {
    m_currentLineup = lineup;
    m_pitchView->setFormation(QString::fromStdString(m_currentLineup.formationName));
    
    for (const auto& playerPos : m_currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::STARTING) {
            setPlayerInPitchView(playerPos.playerId, QString::fromStdString(playerPos.fieldPosition));
        }
    }
    
    populatePlayerLists();
    updateLineupVisibility(true);
    updateLineupRatingDisplay();
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

QListWidget* LineupView::getListWidgetByPosition(const QString& position) const {
    return (position == "RESERVE") ? m_reservesList : m_benchList;
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

void LineupView::saveCurrentLineup() {
    if (m_currentLineup.lineupId <= 0 || !m_currentTeam) {
        QMessageBox::warning(this, "Error", "No lineup selected");
        return;
    }
    
    collectAllPlayerPositions();
    
    m_currentLineup.isActive = true;
    m_teamManager.saveLineup(m_currentLineup);
    loadLineups();
    updateLineupRatingDisplay();
}

void LineupView::collectAllPlayerPositions() {
    m_currentLineup.playerPositions.clear();

    collectPlayerPositionsFromPitch();
    collectPlayersFromList(m_benchList, PositionType::BENCH);
    collectPlayersFromList(m_reservesList, PositionType::RESERVE);

    addRemainingPlayersToReserves();
}

void LineupView::collectPlayerPositionsFromPitch() {
    QMap<QString, int> pitchPlayers = m_pitchView->getPlayersPositions();
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
    m_currentLineup.playerPositions.push_back(pos);
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
    if (!m_currentTeam) {
        return;
    }
    
    QSet<int> processedPlayers;

    for (const auto& pos : m_currentLineup.playerPositions) {
        processedPlayers.insert(pos.playerId);
    }
    
    for (const auto& player : m_currentTeam->players) {
        if (!processedPlayers.contains(player.playerId)) {
            addPlayerToPositions(player.playerId, PositionType::RESERVE, "", 0);
        }
    }
}

void LineupView::handleDragDropPlayer(int playerId, const QString& fromPosition, const QString& toPosition) {
    if (!m_currentTeam) return;
    
    Player* player = findPlayerById(playerId);

    if (!player) {
        return;
    }
    
    processPlayerDragDrop(playerId, fromPosition, toPosition);
    
    saveCurrentLineup();
    updateLineupRatingDisplay();
}

void LineupView::processPlayerDragDrop(int playerId, const QString& fromPosition, const QString& toPosition) {
    bool isFromFieldPos = isFromField(fromPosition);
    bool isToList = (toPosition == "BENCH" || toPosition == "RESERVE");
    
    if (isFromFieldPos && isToList) {
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

int LineupView::findExistingPlayerAtPosition(const QString& position) const {
    QMap<QString, int> currentPositions = m_pitchView->getPlayersPositions();
    
    if (currentPositions.contains(position)) {
        return currentPositions[position];
    }

    return -1;
}

void LineupView::handleFieldToListDrop(int playerId, const QString& fromPosition, const QString& toPosition) {
    m_pitchView->clearPosition(fromPosition);
    
    QListWidget* targetList = (toPosition == "BENCH") ? m_benchList : m_reservesList;
    
    if (!isPlayerInListAlready(playerId, targetList)) {
        addPlayerToList(playerId, targetList);
    }
    
    QTimer::singleShot(100, this, &LineupView::saveCurrentLineup);
}

void LineupView::addPlayerToList(int playerId, QListWidget* targetList) {
    Player* player = findPlayerById(playerId);
    
    if (player) {
        QListWidgetItem* item = createPlayerItem(*player);
        targetList->addItem(item);
    }
}

bool LineupView::isPlayerInListAlready(int playerId, QListWidget* targetList) const {
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
    
    QListWidget* targetList = (toPosition == "BENCH") ? m_benchList : m_reservesList;
    
    if (isFromField(fromPosition)) {
        m_pitchView->clearPosition(fromPosition);
        addPlayerToList(playerId, targetList);
    }
    else {
        movePlayerBetweenLists(playerId, fromPosition, toPosition);
    }
}

bool LineupView::isFromField(const QString& position) const {
    return position != "BENCH" && position != "RESERVE";
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
    
    m_pitchView->setPlayerAtPosition(
        playerId,
        QString::fromStdString(player->name),
        playerImage,
        toPosition
    );
    
    if (isListPosition(fromPosition)) {
        QListWidget* sourceList = (fromPosition == "RESERVE") ? m_reservesList : m_benchList;
        removePlayerFromList(playerId, sourceList);
    }
}

bool LineupView::isListPosition(const QString& position) const {
    return position == "RESERVE" || position == "BENCH";
}

QPixmap LineupView::getPlayerImage(int playerId, const QString& position) {
    if (m_playerImageCache.contains(playerId)) {
        return m_playerImageCache[playerId];
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
            m_playerImageCache[playerId] = playerImage;
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
    QPixmap existingPlayerImage;
    
    if (m_playerImageCache.contains(existingPlayerId)) {
        existingPlayerImage = m_playerImageCache[existingPlayerId];
    }
    
    m_pitchView->setPlayerAtPosition(
        existingPlayerId, 
        QString::fromStdString(existingPlayer->name),
        existingPlayerImage,
        position
    );
}

void LineupView::moveExistingPlayerToList(Player* player, const QString& targetPosition) {
    QListWidget* targetList = (targetPosition == "RESERVE") ? m_reservesList : m_benchList;
    QListWidgetItem* item = createPlayerItem(*player);
    targetList->addItem(item);
}

void LineupView::updatePlayerLists() {
    saveCurrentLineup();
}

void LineupView::populatePlayerLists() {
    if (!m_currentTeam || m_currentLineup.lineupId <= 0) {
        return;
    }

    m_benchList->clear();
    m_reservesList->clear();
    
    std::set<int> teamPlayerIds = getTeamPlayerIds();
    
    QSet<int> usedPlayerIds;
    collectStartingPlayers(m_currentTeam->players, usedPlayerIds);
    collectBenchPlayers(m_currentTeam->players, usedPlayerIds);
    collectReservePlayers(m_currentTeam->players, usedPlayerIds);
    
    addRemainingPlayersToReserveList(usedPlayerIds);
}

void LineupView::collectStartingPlayers(std::span<const Player> teamPlayers, QSet<int>& usedPlayerIds) {
    std::set<int> teamPlayerIds;
    for (const auto& player : teamPlayers) {
        teamPlayerIds.insert(player.playerId);
    }
    
    for (const auto& playerPos : m_currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::STARTING) {
            if (isPlayerInTeam(playerPos.playerId, teamPlayerIds)) {
                usedPlayerIds.insert(playerPos.playerId);
            }
        }
    }
}

void LineupView::collectBenchPlayers(std::span<const Player> teamPlayers, QSet<int>& usedPlayerIds) {
    std::set<int> teamPlayerIds;
    for (const auto& player : teamPlayers) {
        teamPlayerIds.insert(player.playerId);
    }
    
    for (const auto& playerPos : m_currentLineup.playerPositions) {
        if (playerPos.positionType != PositionType::BENCH) {
            continue;
        }
        
        if (!isPlayerInTeam(playerPos.playerId, teamPlayerIds)) {
            continue;
        }
        
        addPlayerToListAndTrack(playerPos.playerId, m_benchList, usedPlayerIds);
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

void LineupView::collectReservePlayers(std::span<const Player> teamPlayers, QSet<int>& usedPlayerIds) {
    std::set<int> teamPlayerIds;
    for (const auto& player : teamPlayers) {
        teamPlayerIds.insert(player.playerId);
    }
    
    for (const auto& playerPos : m_currentLineup.playerPositions) {
        if (playerPos.positionType != PositionType::RESERVE) {
            continue;
        }
        
        if (!isPlayerInTeam(playerPos.playerId, teamPlayerIds)) {
            continue;
        }
        
        addPlayerToListAndTrack(playerPos.playerId, m_reservesList, usedPlayerIds);
    }
}

void LineupView::addRemainingPlayersToReserveList(const QSet<int>& usedPlayerIds) {
    if (!m_currentTeam) {
        return;
    }
    
    for (const auto& player : m_currentTeam->players) {
        if (!usedPlayerIds.contains(player.playerId)) {
            QListWidgetItem* item = createPlayerItem(player);
            m_reservesList->addItem(item);
        }
    }
}

void LineupView::updateLineupVisibility(bool visible) {
    m_lineupStackedWidget->setCurrentIndex(visible ? 1 : 0);
    m_deleteLineupButton->setEnabled(visible);
    m_instructionsLabel->setVisible(visible);
}

QListWidgetItem* LineupView::createPlayerItem(const Player& player) const {
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(QString::fromStdString(player.name));
    item->setData(Qt::UserRole, player.playerId);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    
    item->setToolTip(
        QString("%1 - %2")
        .arg(QString::fromStdString(player.position))
        .arg(QString::fromStdString(player.subPosition))
    );
    
    item->setSelected(false);
    
    return item;
}

Player* LineupView::findPlayerById(int playerId) const {
    if (!m_currentTeam) return nullptr;
    
    for (auto& player : m_currentTeam->players) {
        if (player.playerId == playerId) {
            return &player;
        }
    }
    
    return nullptr;
}

void LineupView::loadPlayerImage(int playerId, const QString& imageUrl, const QString& position) {
    if (imageUrl.isEmpty()) return;
    
    QNetworkRequest request(imageUrl);
    QNetworkReply* reply = m_networkManager->get(request);
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, playerId, position]() {
        this->handleImageLoaded(reply, playerId, position);
    });
}

void LineupView::handleImageLoaded(QNetworkReply* reply, int playerId, const QString& position) {
    if (reply->error() == QNetworkReply::NoError) {
        processLoadedImage(reply, playerId, position);
    }
    
    reply->deleteLater();
}

void LineupView::processLoadedImage(QNetworkReply* reply, int playerId, const QString& position) {
    QPixmap pixmap;
    pixmap.loadFromData(reply->readAll());
    
    m_playerImageCache[playerId] = pixmap;
    
    Player* player = findPlayerById(playerId);
    if (player) {
        m_pitchView->setPlayerAtPosition(
            playerId, 
            QString::fromStdString(player->name),
            pixmap,
            position
        );
    }
}

void LineupView::setupLineupRatingDisplay() {
    m_lineupRatingWidget = new QWidget(this);
    m_lineupRatingWidget->setFixedWidth(200);
    
    auto* ratingLayout = new QVBoxLayout(m_lineupRatingWidget);
    setupRatingLayout(ratingLayout);
    
    m_hasInitialLineupRating = false;
    m_lineupRatingWidget->setVisible(true);
}

void LineupView::setupRatingLayout(QVBoxLayout* ratingLayout) {
    ratingLayout->setContentsMargins(0, 0, 0, 0);
    ratingLayout->setAlignment(Qt::AlignRight | Qt::AlignTop);
    ratingLayout->setSpacing(2);
    
    auto* lineupRatingTitle = new QLabel("Lineup Rating", m_lineupRatingWidget);
    lineupRatingTitle->setStyleSheet("font-size: 12px; color: #aaaaaa;");
    lineupRatingTitle->setAlignment(Qt::AlignRight);
    
    m_lineupRatingLabel = new QLabel("--", m_lineupRatingWidget);
    m_lineupRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white;");
    m_lineupRatingLabel->setAlignment(Qt::AlignRight);
    
    ratingLayout->addWidget(lineupRatingTitle);
    ratingLayout->addWidget(m_lineupRatingLabel);
}

void LineupView::updateLineupRatingDisplay() {
    if (!m_currentTeam || m_currentLineup.lineupId <= 0) {
        resetRatingDisplay();
        return;
    }
    
    double averageRating = calculateCurrentLineupRating();
    
    if (averageRating <= 0) {
        resetRatingDisplay();
        return;
    }
    
    std::pair<int, int> ratingKey = {m_currentTeam->teamId, m_currentLineup.lineupId};

    if (m_initialLineupRatings.find(ratingKey) == m_initialLineupRatings.end()) {
        m_initialLineupRatings[ratingKey] = averageRating;
    }
    
    double ratingDiff = averageRating - m_initialLineupRatings[ratingKey];
    updateRatingLabel(averageRating, ratingDiff);
}

void LineupView::resetRatingDisplay() {
    if (m_lineupRatingLabel) {
        m_lineupRatingLabel->setText("--");
        m_lineupRatingLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: white; text-align: right;");
    }
}

double LineupView::calculateCurrentLineupRating() const {
    double totalRating = 0.0;
    int playerCount = 0;
    
    for (const auto& playerPos : m_currentLineup.playerPositions) {
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

void LineupView::updateRatingLabel(double averageRating, double ratingDiff) {
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
    
    m_lineupRatingLabel->setText(displayText);
    m_lineupRatingLabel->setStyleSheet(styleSheet);
}

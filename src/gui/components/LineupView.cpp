#include "gui/components/LineupView.h"
#include "gui/components/DraggableListWidget.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGroupBox>
#include <QTimer>
#include <QFile>
#include <set>

LineupView::LineupView(TeamManager& tm, Team* currentTeam, QWidget *parent)
    : QWidget(parent)
    , teamManager(tm)
    , currentTeam(currentTeam)
    , playerImageCache()
{
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

    QGridLayout* controlsLayout = new QGridLayout();
    controlsLayout->setContentsMargins(10, 0, 10, 10);
    controlsLayout->setHorizontalSpacing(15);
    controlsLayout->setVerticalSpacing(10);
    
    QLabel* existingLineupsLabel = new QLabel("Lineups:", this);
    existingLineupsComboBox = new QComboBox(this);
    existingLineupsComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    existingLineupsComboBox->setMinimumWidth(150);
    
    newLineupButton = new QPushButton("Create Lineup", this);
    newLineupButton->setEnabled(currentTeam != nullptr);
    deleteLineupButton = new QPushButton("Delete Lineup", this);
    
    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(10);
    buttonsLayout->addWidget(newLineupButton);
    buttonsLayout->addWidget(deleteLineupButton);
    buttonsLayout->addStretch();
    
    controlsLayout->addWidget(existingLineupsLabel, 0, 0);
    controlsLayout->addWidget(existingLineupsComboBox, 0, 1);
    controlsLayout->addLayout(buttonsLayout, 1, 0, 1, 2);
    
    controlsLayout->setColumnStretch(0, 0);
    controlsLayout->setColumnStretch(1, 1);
    
    mainLayout->addLayout(controlsLayout);
    
    lineupNotSelectedLabel = new QLabel("No lineup selected. Create a new lineup or select an existing one.", this);
    lineupNotSelectedLabel->setAlignment(Qt::AlignCenter);
    lineupNotSelectedLabel->setStyleSheet("font-size: 16px; color: #888;");
    
    lineupContentWidget = new QWidget(this);
    QHBoxLayout* contentLayout = new QHBoxLayout(lineupContentWidget);
    contentLayout->setContentsMargins(10, 0, 10, 10);
    
    QVBoxLayout* playersListLayout = new QVBoxLayout();
    playersListLayout->setSpacing(10);
    
    QGroupBox* benchGroup = new QGroupBox("Bench", this);
    QVBoxLayout* benchLayout = new QVBoxLayout(benchGroup);
    benchList = new DraggableListWidget("BENCH", this);
    benchList->setDragEnabled(true);
    benchList->setAcceptDrops(true);
    benchList->setDropIndicatorShown(true);
    benchList->setSelectionMode(QAbstractItemView::SingleSelection);
    benchList->setDragDropMode(QAbstractItemView::DragDrop);
    benchList->setFixedHeight(150);
    benchList->setMinimumWidth(200);
    benchLayout->addWidget(benchList);
    
    QGroupBox* reservesGroup = new QGroupBox("Reserves", this);
    QVBoxLayout* reservesLayout = new QVBoxLayout(reservesGroup);
    reservesList = new DraggableListWidget("RESERVE", this);
    reservesList->setDragEnabled(true);
    reservesList->setAcceptDrops(true);
    reservesList->setDropIndicatorShown(true);
    reservesList->setSelectionMode(QAbstractItemView::SingleSelection);
    reservesList->setDragDropMode(QAbstractItemView::DragDrop);
    reservesList->setMinimumWidth(200);
    reservesLayout->addWidget(reservesList);
    
    playersListLayout->addWidget(benchGroup);
    playersListLayout->addWidget(reservesGroup, 1);
    
    instructionsLabel = new QLabel(this);
    instructionsLabel->setText("Drag players to positions on the field or bench. Click on players to view their stats.");
    instructionsLabel->setStyleSheet("color: #666; font-style: italic;");
    instructionsLabel->setAlignment(Qt::AlignCenter);
    instructionsLabel->setWordWrap(true);
    instructionsLabel->setMaximumHeight(40);
    
    QVBoxLayout* pitchLayout = new QVBoxLayout();
    pitchView = new LineupPitchView(this);
    pitchView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    pitchView->setMinimumHeight(500); 
    
    pitchLayout->addWidget(instructionsLabel);
    pitchLayout->addWidget(pitchView);

    contentLayout->addLayout(playersListLayout, 2);
    contentLayout->addLayout(pitchLayout, 5);

    lineupScrollArea = new QScrollArea(this);
    lineupScrollArea->setWidgetResizable(true);
    lineupScrollArea->setWidget(lineupContentWidget);
    lineupScrollArea->setFrameShape(QFrame::NoFrame);
    
    mainLayout->addWidget(lineupNotSelectedLabel);
    mainLayout->addWidget(lineupScrollArea, 1);
    
    updateLineupVisibility(false);

    benchList->viewport()->installEventFilter(this);
    reservesList->viewport()->installEventFilter(this);
}

bool LineupView::eventFilter(QObject* watched, QEvent* event) {
    if (event->type() == QEvent::Drop) {
        QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
        
        if (watched == benchList->viewport() || watched == reservesList->viewport()) {
            if (dropEvent->mimeData()->hasText()) {
                QString mimeText = dropEvent->mimeData()->text();
                if (mimeText.contains("|")) {
                    QStringList parts = mimeText.split("|");
                    int playerId = parts[0].toInt();
                    QString fromPosition = parts[1];
                    QString toPosition = (watched == benchList->viewport()) ? "BENCH" : "RESERVE";
                    
                    if (fromPosition == toPosition) {
                        return true;
                    }
                    
                    if (fromPosition != "BENCH" && fromPosition != "RESERVE") {
                        Player* player = findPlayerById(playerId);
                        if (player) {
                            pitchView->clearPosition(fromPosition);
                            
                            QListWidget* targetList = (toPosition == "BENCH") ? benchList : reservesList;
                            QListWidgetItem* item = createPlayerItem(*player);
                            targetList->addItem(item);
                            
                            QTimer::singleShot(100, this, &LineupView::updatePlayerLists);
                        }
                        
                        dropEvent->acceptProposedAction();
                        return true;
                    }
                    else if (fromPosition != toPosition) {
                        QListWidget* sourceList = (fromPosition == "RESERVE") ? reservesList : benchList;
                        QListWidget* destList = (toPosition == "BENCH") ? benchList : reservesList;
                        
                        Player* player = findPlayerById(playerId);
                        if (player) {
                            QListWidgetItem* newItem = createPlayerItem(*player);
                            destList->addItem(newItem);
                            
                            for (int i = 0; i < sourceList->count(); ++i) {
                                QListWidgetItem* item = sourceList->item(i);
                                if (item->data(Qt::UserRole).toInt() == playerId) {
                                    delete sourceList->takeItem(i);
                                    break;
                                }
                            }
                            
                            dropEvent->acceptProposedAction();
                        }
                    }
                    
                    QTimer::singleShot(100, this, &LineupView::updatePlayerLists);
                    return true;
                }
            }
        }
    }
    
    return QWidget::eventFilter(watched, event);
}

void LineupView::setupConnections() {
    connect(existingLineupsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LineupView::loadSelectedLineup);
            
    connect(newLineupButton, &QPushButton::clicked, 
            this, &LineupView::createNewLineup);
            
    connect(deleteLineupButton, &QPushButton::clicked, [this]() {
        if (currentLineup.lineupId <= 0 || !currentTeam) {
            return;
        }
        
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Delete Lineup", "Are you sure you want to delete this lineup?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            teamManager.deleteLineup(currentLineup.lineupId);
            loadLineups();
            updateLineupVisibility(false);
        }
    });
    
    connect(pitchView, &LineupPitchView::playerDragDropped,
            this, &LineupView::handleDragDropPlayer);
            
    connect(pitchView, &LineupPitchView::playerClicked,
            this, &LineupView::playerClicked);
            
    connect(benchList, &QListWidget::itemChanged, [this]() {
        updatePlayerLists();
    });
    
    connect(reservesList, &QListWidget::itemChanged, [this]() {
        updatePlayerLists();
    });
    
    connect(benchList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        if (item) {
            int playerId = item->data(Qt::UserRole).toInt();
            benchList->clearSelection();
            emit playerClicked(playerId);
        }
    });

    connect(reservesList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        if (item) {
            int playerId = item->data(Qt::UserRole).toInt();
            reservesList->clearSelection();
            emit playerClicked(playerId);
        }
    });
    
    connect(dynamic_cast<DraggableListWidget*>(benchList), &DraggableListWidget::fieldPlayerDropped,
        [this](int playerId, const QString& fromPosition, const QString& toListType) {
            handleDragDropPlayer(playerId, fromPosition, toListType);
        });

    connect(dynamic_cast<DraggableListWidget*>(reservesList), &DraggableListWidget::fieldPlayerDropped,
        [this](int playerId, const QString& fromPosition, const QString& toListType) {
            handleDragDropPlayer(playerId, fromPosition, toListType);
        });
}

void LineupView::loadLineups() {
    if (!currentTeam) {
        return;
    }
    
    existingLineupsComboBox->clear();
    existingLineupsComboBox->addItem("-- Select Lineup --", -1);
    
    std::vector<Lineup> lineups = teamManager.getTeamLineups(currentTeam->teamId);
    
    for (const auto& lineup : lineups) {
        QString formationName = QString::fromStdString(lineup.formationName);
        QString lineupName = QString::fromStdString(lineup.name);
        
        QString displayName;
        if (!lineupName.isEmpty()) {
            displayName = lineupName + " (" + formationName + ")";
        } else {
            displayName = QString("Lineup %1 (%2)").arg(lineup.lineupId).arg(formationName);
        }
        
        existingLineupsComboBox->addItem(displayName, lineup.lineupId);
        
        if (lineup.isActive) {
            int idx = existingLineupsComboBox->count() - 1;
            QFont font = existingLineupsComboBox->itemData(idx, Qt::FontRole).value<QFont>();
            font.setBold(true);
            existingLineupsComboBox->setItemData(idx, font, Qt::FontRole);
            existingLineupsComboBox->setCurrentIndex(idx);
        }
    }
}

void LineupView::setTeam(Team* team) {
    currentTeam = team;
    currentLineup = Lineup();
    
    playerImageCache.clear();
    
    pitchView->clearPositions();
    
    if (currentTeam) {
        for (const auto& player : currentTeam->players) {
            QString imageUrl = QString::fromStdString(player.imageUrl);
            if (imageUrl.contains(",")) {
                imageUrl = imageUrl.split(",").first().trimmed();
            }
            
            if (!imageUrl.isEmpty() && !imageUrl.startsWith("http")) {
                QPixmap playerImage;
                if (playerImage.load(imageUrl)) {
                    QPixmap scaledImage = playerImage.scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    playerImageCache[player.playerId] = scaledImage;
                }
            }
        }
        
        loadLineups();
        
        Lineup activeLineup = teamManager.getActiveLineup(currentTeam->teamId);
        if (activeLineup.lineupId > 0) {
            currentLineup = activeLineup;
            pitchView->setFormation(QString::fromStdString(currentLineup.formationName));
            
            std::set<int> teamPlayerIds;
            for (const auto& player : currentTeam->players) {
                teamPlayerIds.insert(player.playerId);
            }
            
            for (const auto& playerPos : currentLineup.playerPositions) {
                if (playerPos.positionType == PositionType::STARTING) {
                    if (teamPlayerIds.find(playerPos.playerId) == teamPlayerIds.end()) {
                        continue;
                    }
                    
                    Player* player = findPlayerById(playerPos.playerId);
                    if (player) {
                        QPixmap playerImage;
                        if (playerImageCache.contains(playerPos.playerId)) {
                            playerImage = playerImageCache[playerPos.playerId];
                        } else {
                            QString imageUrl = QString::fromStdString(player->imageUrl);
                            if (imageUrl.contains(",")) {
                                imageUrl = imageUrl.split(",").first().trimmed();
                            }
                            
                            if (!imageUrl.isEmpty()) {
                                if (imageUrl.startsWith("http")) {
                                    loadPlayerImage(playerPos.playerId, imageUrl, 
                                        QString::fromStdString(playerPos.fieldPosition));
                                }
                            }
                        }
                        
                        pitchView->setPlayerAtPosition(
                            playerPos.playerId,
                            QString::fromStdString(player->name),
                            playerImage,
                            QString::fromStdString(playerPos.fieldPosition)
                        );
                    }
                }
            }
            
            populatePlayerLists();
            updateLineupVisibility(true);
        } else {
            updateLineupVisibility(false);
        }
    } else {
        updateLineupVisibility(false);
    }
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
        
        Lineup newLineup = teamManager.createLineup(currentTeam->teamId, formationId, lineupName.toStdString());
        currentLineup = newLineup;
        
        pitchView->setFormation(QString::fromStdString(currentLineup.formationName));
        populatePlayerLists();
        loadLineups();
        
        for (int i = 0; i < existingLineupsComboBox->count(); ++i) {
            if (existingLineupsComboBox->itemData(i).toInt() == newLineup.lineupId) {
                existingLineupsComboBox->setCurrentIndex(i);
                break;
            }
        }
        
        updateLineupVisibility(true);
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
    
    std::vector<Lineup> lineups = teamManager.getTeamLineups(currentTeam->teamId);
    for (const auto& lineup : lineups) {
        if (lineup.lineupId == lineupId) {
            currentLineup = lineup;
            pitchView->setFormation(QString::fromStdString(currentLineup.formationName));
            
            for (const auto& playerPos : currentLineup.playerPositions) {
                if (playerPos.positionType == PositionType::STARTING) {
                    Player* player = findPlayerById(playerPos.playerId);
                    if (player) {
                        QPixmap playerImage;
                        QString imageUrl = QString::fromStdString(player->imageUrl);
                        if (imageUrl.contains(",")) {
                            imageUrl = imageUrl.split(",").first().trimmed();
                        }
                        
                        if (playerImageCache.contains(playerPos.playerId)) {
                            playerImage = playerImageCache[playerPos.playerId];
                        } else if (!imageUrl.isEmpty()) {
                            if (imageUrl.startsWith("http")) {
                                loadPlayerImage(playerPos.playerId, imageUrl, 
                                    QString::fromStdString(playerPos.fieldPosition));
                            } else {
                                playerImage.load(imageUrl);
                                playerImageCache[playerPos.playerId] = playerImage;
                            }
                        }
                        
                        pitchView->setPlayerAtPosition(
                            playerPos.playerId, 
                            QString::fromStdString(player->name),
                            playerImage,
                            QString::fromStdString(playerPos.fieldPosition)
                        );
                    } else {
                        pitchView->setPlayerAtPosition(playerPos.playerId, 
                            QString::fromStdString(playerPos.fieldPosition));
                    }
                }
            }
            
            populatePlayerLists();
            updateLineupVisibility(true);
            return;
        }
    }
}

void LineupView::saveCurrentLineup(bool forceSave) {
    if (currentLineup.lineupId <= 0 || !currentTeam) {
        QMessageBox::warning(this, "Error", "No lineup selected");
        return;
    }
    
    currentLineup.playerPositions.clear();
    
    QMap<QString, int> pitchPlayers = pitchView->getPlayersPositions();
    QSet<int> processedPlayers;
    
    for (auto it = pitchPlayers.begin(); it != pitchPlayers.end(); ++it) {
        if (it.value() > 0) {
            PlayerPosition pos;
            pos.playerId = it.value();
            pos.positionType = PositionType::STARTING;
            pos.fieldPosition = it.key().toStdString();
            pos.order = 0;
            currentLineup.playerPositions.push_back(pos);
            processedPlayers.insert(pos.playerId);
        }
    }
    
    for (int i = 0; i < benchList->count(); ++i) {
        QListWidgetItem* item = benchList->item(i);
        int playerId = item->data(Qt::UserRole).toInt();
        
        if (!processedPlayers.contains(playerId)) {
            PlayerPosition pos;
            pos.playerId = playerId;
            pos.positionType = PositionType::BENCH;
            pos.fieldPosition = "";
            pos.order = i;
            currentLineup.playerPositions.push_back(pos);
            processedPlayers.insert(playerId);
        }
    }
    
    for (int i = 0; i < reservesList->count(); ++i) {
        QListWidgetItem* item = reservesList->item(i);
        int playerId = item->data(Qt::UserRole).toInt();
        
        if (!processedPlayers.contains(playerId)) {
            PlayerPosition pos;
            pos.playerId = playerId;
            pos.positionType = PositionType::RESERVE;
            pos.fieldPosition = "";
            pos.order = i;
            currentLineup.playerPositions.push_back(pos);
            processedPlayers.insert(playerId);
        }
    }
    
    for (const auto& player : currentTeam->players) {
        if (!processedPlayers.contains(player.playerId)) {
            PlayerPosition pos;
            pos.playerId = player.playerId;
            pos.positionType = PositionType::RESERVE;
            pos.fieldPosition = "";
            pos.order = 0;
            currentLineup.playerPositions.push_back(pos);
        }
    }
    
    currentLineup.isActive = true;
    
    teamManager.saveLineup(currentLineup);
    
    loadLineups();
}

void LineupView::handleDragDropPlayer(int playerId, const QString& fromPosition, const QString& toPosition) {
    if (!currentTeam) return;
    
    Player* player = findPlayerById(playerId);
    if (!player) {
        return;
    }
    
    bool isFromField = (fromPosition != "BENCH" && fromPosition != "RESERVE");
    bool isToList = (toPosition == "BENCH" || toPosition == "RESERVE");
    
    if (isFromField && isToList) {
        pitchView->clearPosition(fromPosition);
        
        QListWidget* targetList = (toPosition == "BENCH") ? benchList : reservesList;
        
        bool alreadyInList = false;
        for (int i = 0; i < targetList->count(); i++) {
            if (targetList->item(i)->data(Qt::UserRole).toInt() == playerId) {
                alreadyInList = true;
                break;
            }
        }
        
        if (!alreadyInList) {
            QListWidgetItem* item = createPlayerItem(*player);
            targetList->addItem(item);
        }
        
        QTimer::singleShot(100, this, [this]() {
            saveCurrentLineup(true);
        });
        
        return;
    }
    
    int existingPlayerId = -1;
    QMap<QString, int> currentPositions = pitchView->getPlayersPositions();
    if (currentPositions.contains(toPosition)) {
        existingPlayerId = currentPositions[toPosition];
    }
    
    if (toPosition == "BENCH" || toPosition == "RESERVE") {
        QListWidget* targetList = (toPosition == "BENCH") ? benchList : reservesList;
        
        if (fromPosition != "BENCH" && fromPosition != "RESERVE") {
            pitchView->clearPosition(fromPosition);
            
            QListWidgetItem* item = createPlayerItem(*player);
            targetList->addItem(item);
        }
        else if (fromPosition != toPosition) {
            QListWidget* sourceList = (fromPosition == "BENCH") ? benchList : reservesList;
            
            for (int i = 0; i < sourceList->count(); ++i) {
                QListWidgetItem* item = sourceList->item(i);
                if (item->data(Qt::UserRole).toInt() == playerId) {
                    delete sourceList->takeItem(i);
                    break;
                }
            }
            
            QListWidgetItem* item = createPlayerItem(*player);
            targetList->addItem(item);
        }
    }
    else {
        if (existingPlayerId > 0 && existingPlayerId != playerId) {
            Player* existingPlayer = findPlayerById(existingPlayerId);
            
            if (fromPosition != "RESERVE" && fromPosition != "BENCH") {
                if (existingPlayer) {
                    QPixmap existingPlayerImage;
                    if (playerImageCache.contains(existingPlayerId)) {
                        existingPlayerImage = playerImageCache[existingPlayerId];
                    }
                    
                    pitchView->setPlayerAtPosition(
                        existingPlayerId, 
                        QString::fromStdString(existingPlayer->name),
                        existingPlayerImage,
                        fromPosition
                    );
                }
            } 
            else {
                QListWidget* targetList = (fromPosition == "RESERVE") ? reservesList : benchList;
                
                if (existingPlayer) {
                    QListWidgetItem* item = createPlayerItem(*existingPlayer);
                    targetList->addItem(item);
                }
            }
        }
        
        QPixmap playerImage;
        if (playerImageCache.contains(playerId)) {
            playerImage = playerImageCache[playerId];
        } else {
            QString imageUrl = QString::fromStdString(player->imageUrl);
            if (imageUrl.contains(",")) {
                imageUrl = imageUrl.split(",").first().trimmed();
            }
            
            if (!imageUrl.isEmpty()) {
                if (imageUrl.startsWith("http")) {
                    loadPlayerImage(playerId, imageUrl, toPosition);
                } else if (QFile::exists(imageUrl)) {
                    playerImage.load(imageUrl);
                    playerImageCache[playerId] = playerImage;
                }
            }
        }
        
        pitchView->setPlayerAtPosition(
            playerId,
            QString::fromStdString(player->name),
            playerImage,
            toPosition
        );
        
        if (fromPosition == "RESERVE" || fromPosition == "BENCH") {
            QListWidget* sourceList = (fromPosition == "RESERVE") ? reservesList : benchList;
            
            for (int i = 0; i < sourceList->count(); ++i) {
                QListWidgetItem* item = sourceList->item(i);
                if (item->data(Qt::UserRole).toInt() == playerId) {
                    delete sourceList->takeItem(i);
                    break;
                }
            }
        }
    }
    
    saveCurrentLineup(true);
}

void LineupView::updatePlayerLists() {
    saveCurrentLineup(true);
}

void LineupView::populatePlayerLists() {
    if (!currentTeam || currentLineup.lineupId <= 0) {
        return;
    }

    benchList->clear();
    reservesList->clear();
    
    std::set<int> teamPlayerIds;
    for (const auto& player : currentTeam->players) {
        teamPlayerIds.insert(player.playerId);
    }
    
    QSet<int> usedPlayerIds;
    
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::STARTING) {
            if (teamPlayerIds.find(playerPos.playerId) != teamPlayerIds.end()) {
                usedPlayerIds.insert(playerPos.playerId);
            }
        }
    }
    
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::BENCH) {
            if (teamPlayerIds.find(playerPos.playerId) == teamPlayerIds.end()) {
                continue;
            }
            
            Player* player = findPlayerById(playerPos.playerId);
            if (player) {
                QListWidgetItem* item = createPlayerItem(*player);
                benchList->addItem(item);
                usedPlayerIds.insert(playerPos.playerId);
            }
        }
    }
    
    for (const auto& playerPos : currentLineup.playerPositions) {
        if (playerPos.positionType == PositionType::RESERVE) {
            if (teamPlayerIds.find(playerPos.playerId) == teamPlayerIds.end()) {
                continue;
            }
            
            Player* player = findPlayerById(playerPos.playerId);
            if (player) {
                QListWidgetItem* item = createPlayerItem(*player);
                reservesList->addItem(item);
                usedPlayerIds.insert(playerPos.playerId);
            }
        }
    }
    
    for (const auto& player : currentTeam->players) {
        if (!usedPlayerIds.contains(player.playerId)) {
            QListWidgetItem* item = createPlayerItem(player);
            reservesList->addItem(item);
        }
    }
}

void LineupView::updateLineupVisibility(bool visible) {
    lineupNotSelectedLabel->setVisible(!visible);
    lineupScrollArea->setVisible(visible);
    deleteLineupButton->setEnabled(visible);
    instructionsLabel->setVisible(visible);
}

QListWidgetItem* LineupView::createPlayerItem(const Player& player) {
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(QString::fromStdString(player.name));
    item->setData(Qt::UserRole, player.playerId);
    item->setFlags(item->flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
    
    item->setToolTip(QString("%1 - %2")
                     .arg(QString::fromStdString(player.position))
                     .arg(QString::fromStdString(player.subPosition)));
    
    item->setSelected(false);
    
    return item;
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
    
    connect(reply, &QNetworkReply::finished, this, [this, reply, playerId, position]() {
        this->handleImageLoaded(reply, playerId, position);
    });
}

void LineupView::handleImageLoaded(QNetworkReply* reply, int playerId, const QString& position) {
    if (reply->error() == QNetworkReply::NoError) {
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
    
    reply->deleteLater();
    sender()->deleteLater();
}

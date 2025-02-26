#include "gui/views/TeamManagerView.h"
#include "gui/models/Models.h"
#include "gui/components/ClubSelectDialog.h"
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
#include <iostream>

TeamManagerView::TeamManagerView(TeamManager& tm, QWidget *parent)
    : QWidget(parent)
    , teamManager(tm)
    , model(new TeamListModel(teamManager))
    , currentTeam(nullptr)
    , networkManager(new QNetworkAccessManager(this))
{
    setupUi();
    setupConnections();

    teamManager.loadTeams();
    model->refresh();
    updateTeamInfo();
}

void TeamManagerView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    backButton = new QPushButton("Back to Menu", this);
    mainLayout->addWidget(backButton, 0, Qt::AlignLeft);

    QHBoxLayout* mainContentLayout = new QHBoxLayout();
    mainContentLayout->setSpacing(10);

    QVBoxLayout* leftLayout = new QVBoxLayout();
    QLabel* teamsLabel = new QLabel("Existing Teams:", this);
    teamList = new QListView(this);
    teamList->setModel(model);
    teamList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHBoxLayout* newTeamLayout = new QHBoxLayout();
    teamNameInput = new QLineEdit(this);
    teamNameInput->setPlaceholderText("New Team Name");
    newTeamButton = new QPushButton("Create Team", this);
    newTeamLayout->addWidget(teamNameInput);
    newTeamLayout->addWidget(newTeamButton);

    QHBoxLayout* loadTeamLayout = new QHBoxLayout();
    loadTeamByIdButton = new QPushButton("Load Team from Club", this);
    loadTeamLayout->addWidget(loadTeamByIdButton);
    deleteTeamButton = new QPushButton("Delete Team", this);

    editTeamNameButton = new QPushButton("Edit Team Name", this);
    
    leftLayout->addWidget(teamsLabel);
    leftLayout->addWidget(teamList);
    leftLayout->addLayout(newTeamLayout);
    leftLayout->addLayout(loadTeamLayout);
    leftLayout->addWidget(deleteTeamButton);
    leftLayout->addWidget(editTeamNameButton);

    QVBoxLayout* centerLayout = new QVBoxLayout();
    QLabel* currentTeamLabel = new QLabel("Current Team:", this);
    
    currentTeamPlayers = new QTableView(this);
    currentTeamPlayers->setShowGrid(false);
    currentTeamPlayers->setSelectionBehavior(QAbstractItemView::SelectRows);
    currentTeamPlayers->setSelectionMode(QAbstractItemView::SingleSelection);
    currentTeamPlayers->verticalHeader()->setVisible(false);
    currentTeamPlayers->horizontalHeader()->setVisible(false);
    currentTeamPlayers->setIconSize(QSize(32, 32));
    currentTeamPlayers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    currentTeamPlayers->setStyleSheet(
        "QTableView {"
        "  background-color: #2d2d2d;"
        "  border: 1px solid #404040;"
        "  border-radius: 4px;"
        "  padding: 0px;"
        "}"
        "QTableView::item {"
        "  padding: 5px;"
        "  border-bottom: 1px solid #333333;"
        "}"
        "QTableView::item:selected {"
        "  background-color: #0078d4;"
        "  color: white;"
        "}"
    );

    QHBoxLayout* budgetLayout = new QHBoxLayout();
    QLabel* budgetLabel = new QLabel("Budget (€):", this);
    budgetInput = new QSpinBox(this);
    budgetInput->setRange(0, 1000000000);
    budgetInput->setValue(20000000);
    budgetInput->setSingleStep(1000000);
    budgetLayout->addWidget(budgetLabel);
    budgetLayout->addWidget(budgetInput);

    autoFillButton = new QPushButton("Auto-Fill Team", this);
    removePlayerButton = new QPushButton("Remove Player", this);

    centerLayout->addWidget(currentTeamLabel);
    centerLayout->addWidget(currentTeamPlayers);
    centerLayout->addLayout(budgetLayout);
    centerLayout->addWidget(autoFillButton);
    centerLayout->addWidget(removePlayerButton);

    QVBoxLayout* rightLayout = new QVBoxLayout();
    QLabel* playerDetailsLabel = new QLabel("Selected Player:", this);
    playerImage = new QLabel(this);
    playerImage->setFixedSize(200, 200);
    playerImage->setAlignment(Qt::AlignCenter);

    playerName = new QLabel("", this);
    playerClub = new QLabel("", this);
    playerPosition = new QLabel("", this);
    playerMarketValue = new QLabel("", this);
    playerRating = new QLabel("", this);

    rightLayout->addWidget(playerDetailsLabel);
    rightLayout->addWidget(playerImage);
    rightLayout->addWidget(playerName);
    rightLayout->addWidget(playerClub);
    rightLayout->addWidget(playerPosition);
    rightLayout->addWidget(playerMarketValue);
    rightLayout->addWidget(playerRating);
    rightLayout->addStretch();

    mainContentLayout->addLayout(leftLayout, 1);
    mainContentLayout->addLayout(centerLayout, 1);
    mainContentLayout->addLayout(rightLayout, 1);
    mainLayout->addLayout(mainContentLayout);
}

void TeamManagerView::setupConnections() {
    connect(newTeamButton, &QPushButton::clicked, this, &TeamManagerView::createNewTeam);
    connect(loadTeamByIdButton, &QPushButton::clicked, this, &TeamManagerView::loadTeamById);
    connect(autoFillButton, &QPushButton::clicked, this, &TeamManagerView::autoFillTeam);
    connect(budgetInput, QOverload<int>::of(&QSpinBox::valueChanged), this, &TeamManagerView::updateBudget);
    connect(removePlayerButton, &QPushButton::clicked, this, &TeamManagerView::removeSelectedPlayer);
    connect(backButton, &QPushButton::clicked, this, &TeamManagerView::navigateBack);
    connect(deleteTeamButton, &QPushButton::clicked, this, &TeamManagerView::deleteSelectedTeam);
    connect(editTeamNameButton, &QPushButton::clicked, this, &TeamManagerView::editTeamName);

    if (teamList->selectionModel()) { 
        connect(teamList->selectionModel(), &QItemSelectionModel::currentChanged, this, &TeamManagerView::loadSelectedTeam);
    } else {
        qWarning() << "Warning: `teamList` selection model is null.";
    }

    currentTeamPlayers->setEnabled(false);
    autoFillButton->setEnabled(false);
    budgetInput->setEnabled(false);
    removePlayerButton->setEnabled(false);
    editTeamNameButton->setEnabled(false);
}

void TeamManagerView::createNewTeam() {
    QString name = teamNameInput->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter a team name");
        return;
    }

    try {
        Team& newTeam = teamManager.createTeam(name.toStdString());
        currentTeam = &newTeam;
        teamManager.saveTeam(newTeam);
        model->refresh();
        updateTeamInfo();
        
        currentTeamPlayers->setEnabled(true);
        autoFillButton->setEnabled(true);
        budgetInput->setEnabled(true);
        removePlayerButton->setEnabled(true);
        editTeamNameButton->setEnabled(true);

        teamNameInput->clear();
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to create team: %1").arg(e.what()));
        currentTeam = nullptr;
    }
}

void TeamManagerView::loadSelectedTeam() {
    QModelIndex index = teamList->currentIndex();
    if (!index.isValid()) return;

    try {
        int teamId = model->data(index, Qt::UserRole).toInt();
        Team& selectedTeam = teamManager.loadTeam(teamId);
        currentTeam = &selectedTeam;
        updateTeamInfo();
        
        editTeamNameButton->setEnabled(true);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to load team: %1").arg(e.what()));
        currentTeam = nullptr;
        editTeamNameButton->setEnabled(false);
    }
}

void TeamManagerView::loadTeamById() {
    if (availableClubs.empty()) {
        try {
            availableClubs = teamManager.getAllClubs();
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error", QString("Failed to load clubs: %1").arg(e.what()));
            return;
        }
    }
    
    ClubSelectDialog dialog(availableClubs, this);
    if (dialog.exec() == QDialog::Accepted) {
        int clubId = dialog.getSelectedClubId();
        if (clubId != -1) {
            try {
                Team& selectedTeam = teamManager.loadTeamFromClub(clubId);
                currentTeam = &selectedTeam;
                teamManager.saveTeam(selectedTeam);
                model->refresh();
                updateTeamInfo();
                editTeamNameButton->setEnabled(true);
            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error", QString("Failed to load team: %1").arg(e.what()));
                currentTeam = nullptr;
                editTeamNameButton->setEnabled(false);
            }
        }
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
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to auto-fill team: %1").arg(e.what()));
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

void TeamManagerView::removeSelectedPlayer() {
    QModelIndex index = currentTeamPlayers->currentIndex();
    if (!index.isValid() || !currentTeam) return;

    QModelIndex playerIdIndex = currentTeamPlayers->model()->index(index.row(), 0);
    int playerId = playerIdIndex.data(Qt::UserRole).toInt();

    if (playerId == 0) {
        QMessageBox::warning(this, "Error", "Invalid player selected.");
        return;
    }

    teamManager.removePlayerFromTeam(currentTeam->teamId, playerId);
    teamManager.saveTeamPlayers(*currentTeam);
    updateTeamInfo();
}

void TeamManagerView::editTeamName() {
    QModelIndex index = teamList->currentIndex();
    if (!index.isValid() || !currentTeam) {
        QMessageBox::warning(this, "Error", "Please select a team first");
        return;
    }

    bool ok;
    QString currentName = QString::fromStdString(currentTeam->teamName);
    QString newName = QInputDialog::getText(this, 
                                         "Edit Team Name", 
                                         "Enter new team name:", 
                                         QLineEdit::Normal,
                                         currentName, 
                                         &ok);

    if (ok && !newName.isEmpty()) {
        try {
            if (teamManager.updateTeamName(currentTeam->teamId, newName.toStdString())) {
                model->refresh();
                
                for(int i = 0; i < model->rowCount(); i++) {
                    QModelIndex idx = model->index(i, 0);
                    if (model->data(idx, Qt::UserRole).toInt() == currentTeam->teamId) {
                        teamList->setCurrentIndex(idx);
                        break;
                    }
                }
            } else {
                QMessageBox::critical(this, "Error", "Failed to update team name");
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to update team name: %1").arg(e.what()));
        }
    }
}

void TeamManagerView::updateTeamInfo() {
    if (!currentTeam) {
        currentTeamPlayers->setModel(nullptr);
        currentTeamPlayers->setEnabled(false);
        budgetInput->setEnabled(false);
        autoFillButton->setEnabled(false);
        removePlayerButton->setEnabled(false);
        editTeamNameButton->setEnabled(false);
        return;
    }

    QAbstractItemModel* oldModel = currentTeamPlayers->model();
    QStandardItemModel* playerModel = new QStandardItemModel(this);
    playerModel->setColumnCount(2);

    for (const auto& player : currentTeam->players) {
        QStandardItem* imageItem = new QStandardItem();
        QStandardItem* nameItem = new QStandardItem(QString::fromStdString(player.name));
        
        imageItem->setEditable(false);
        nameItem->setEditable(false);
        
        imageItem->setData(player.playerId, Qt::UserRole);
        nameItem->setData(player.playerId, Qt::UserRole);
        
        if (playerImageCache.contains(player.playerId)) {
            imageItem->setIcon(QIcon(playerImageCache[player.playerId]));
        } else {
            QString imageUrl = QString::fromStdString(player.imageUrl);
            if (imageUrl.contains(",")) {
                imageUrl = imageUrl.split(",").first().trimmed(); 
            }
            
            if (!imageUrl.isEmpty()) {
                loadPlayerImage(player.playerId, imageUrl);
            }
        }
        
        playerModel->appendRow({imageItem, nameItem});
    }
    
    currentTeamPlayers->setModel(playerModel);
    currentTeamPlayers->setColumnWidth(0, 40);
    currentTeamPlayers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    
    currentTeamPlayers->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    if (currentTeamPlayers->selectionModel()) {
        disconnect(currentTeamPlayers->selectionModel(), nullptr, this, nullptr);
        connect(currentTeamPlayers->selectionModel(), &QItemSelectionModel::currentChanged, 
                this, &TeamManagerView::updatePlayerDetails);
    }

    if (oldModel && oldModel != playerModel) {
        delete oldModel;
    }

    currentTeamPlayers->setEnabled(true);
    budgetInput->setEnabled(true);
    budgetInput->setValue(currentTeam->budget);
    autoFillButton->setEnabled(true);
    removePlayerButton->setEnabled(true);
    editTeamNameButton->setEnabled(true);
}

void TeamManagerView::updatePlayerDetails() {
    QModelIndex index = currentTeamPlayers->currentIndex();
    if (!index.isValid() || !currentTeam) return;

    QModelIndex playerIdIndex = currentTeamPlayers->model()->index(index.row(), 0);
    int playerId = playerIdIndex.data(Qt::UserRole).toInt();
    
    for (const auto& player : currentTeam->players) {
        if (player.playerId == playerId) {
            playerName->setText("Name: " + QString::fromStdString(player.name));
            playerClub->setText("Club: " + QString::fromStdString(player.clubName));
            playerPosition->setText("Position: " + QString::fromStdString(player.position));
            playerMarketValue->setText("Market Value: €" + QString::number(player.marketValue / 1000000) + "M");
            playerRating->setText("Rating: " + QString::number(player.rating));

            QString imageUrl = QString::fromStdString(player.imageUrl);
            if (imageUrl.contains(",")) {
                imageUrl = imageUrl.split(",").first().trimmed(); 
            }

            if (imageUrl.startsWith("http")) {
                fetchPlayerDetailImage(imageUrl);
            } else {
                loadLocalPlayerDetailImage(imageUrl);
            }
            break;
        }
    }
}

void TeamManagerView::loadPlayerImage(int playerId, const QString& imageUrl) {
    if (imageUrl.isEmpty()) return;
    
    if (imageUrl.startsWith("http")) {
        QUrl url(imageUrl);
        QNetworkRequest request(url);
        QNetworkReply* reply = networkManager->get(request);
        
        connect(reply, &QNetworkReply::finished, this, [this, reply, playerId]() {
            this->handleImageResponse(reply, playerId);
        });
    } else {
        loadLocalImage(playerId, imageUrl);
    }
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

void TeamManagerView::loadLocalPlayerDetailImage(const QString& imageUrl) {
    QPixmap pixmap;
    if (pixmap.load(imageUrl)) {
        playerImage->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatio));
    } else {
        playerImage->setText("No Image");
    }
}

void TeamManagerView::deleteSelectedTeam() {
    QModelIndex index = teamList->currentIndex();
    if (!index.isValid()) return;

    int teamId = model->data(index, Qt::UserRole).toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Team", "Are you sure you want to delete this team?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        try {
            teamManager.deleteTeam(teamId);
            model->refresh();
            currentTeam = nullptr;
            updateTeamInfo();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", QString("Failed to delete team: %1").arg(e.what()));
        }
    }
}

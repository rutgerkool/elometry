#include "gui/views/PlayerListView.h"
#include "gui/models/PlayerListModel.h"
#include "gui/components/PlayerHistoryDialog.h"
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

PlayerListView::PlayerListView(RatingManager& rm, QWidget *parent)
    : QWidget(parent)
    , ratingManager(rm)
    , model(new PlayerListModel(ratingManager.getSortedRatedPlayers()))
    , networkManager(new QNetworkAccessManager(this))
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

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel("Search:", this);
    searchLabel->setProperty("section", "search");
    searchBox = new QLineEdit(this);
    searchBox->setObjectName("searchBox");
    searchBox->setFixedWidth(240);
    searchBox->setPlaceholderText("Search players");
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchBox);
    searchLayout->addStretch();

    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* positionLabel = new QLabel("Position:", this);
    positionLabel->setProperty("section", "position");
    positionFilter = new QComboBox(this);
    positionFilter->setObjectName("positionFilter");
    positionFilter->addItem("All Positions");
    positionFilter->addItems({"Goalkeeper", "Defender", "Midfield", "Attack"});
    positionFilter->setFixedWidth(240);
    filterLayout->addWidget(positionLabel);
    filterLayout->addWidget(positionFilter);
    filterLayout->addStretch();

    inputLayout->addLayout(searchLayout);
    inputLayout->addSpacing(20);
    inputLayout->addLayout(filterLayout);
    inputLayout->addStretch();

    mainLayout->addLayout(inputLayout);

    QHBoxLayout* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(20);

    QFrame* tableFrame = new QFrame(this);
    tableFrame->setFrameShape(QFrame::NoFrame);
    tableFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout* tableLayout = new QVBoxLayout(tableFrame);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableLayout->setSpacing(10);

    tableView = new QTableView(tableFrame);
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

    tableLayout->addWidget(tableView, 1);

    QWidget* paginationWidget = new QWidget(tableFrame);
    paginationWidget->setFixedHeight(50);
    QHBoxLayout* paginationLayout = new QHBoxLayout(paginationWidget);
    paginationLayout->setContentsMargins(0, 0, 0, 0);
    
    prevPageButton = new QPushButton("Previous", paginationWidget);
    prevPageButton->setObjectName("prevPageButton");
    
    QWidget* pageInfoContainer = new QWidget(paginationWidget);
    QHBoxLayout* pageInfoLayout = new QHBoxLayout(pageInfoContainer);
    pageInfoLayout->setContentsMargins(0, 0, 0, 0);
    
    pageInfoLabel = new QLabel("Page 1", pageInfoContainer);
    pageInfoLabel->setObjectName("pageInfoLabel");
    totalPagesLabel = new QLabel("/ 1", pageInfoContainer);
    totalPagesLabel->setObjectName("totalPagesLabel");
    
    pageInfoLayout->addStretch(1);
    pageInfoLayout->addWidget(pageInfoLabel);
    pageInfoLayout->addWidget(totalPagesLabel);
    pageInfoLayout->addStretch(1);
    
    nextPageButton = new QPushButton("Next", paginationWidget);
    nextPageButton->setObjectName("nextPageButton");
    
    paginationLayout->addWidget(prevPageButton);
    paginationLayout->addWidget(pageInfoContainer);
    paginationLayout->addWidget(nextPageButton);
    paginationLayout->addStretch();
    
    tableLayout->addWidget(paginationWidget, 0);

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
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* playerDetailsLabel = new QLabel("Selected Player:", this);
    playerDetailsLabel->setObjectName("playerDetailsLabel");

    playerImage = new QLabel();
    playerImage->setObjectName("playerImage");
    playerImage->setFixedSize(200, 200);
    playerImage->setAlignment(Qt::AlignCenter);
    playerImage->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    playerName = new QLabel("");
    playerName->setObjectName("playerName");
    playerName->setWordWrap(true);
    playerName->setAlignment(Qt::AlignLeft);
    
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
    
    playerRating = new QLabel("");
    playerRating->setObjectName("playerRating");
    playerRating->setWordWrap(true);
    playerRating->setAlignment(Qt::AlignLeft);
    
    viewHistoryButton = new QPushButton("View Rating History", this);
    viewHistoryButton->setObjectName("viewHistoryButton");
    viewHistoryButton->setEnabled(false);
    
    QHBoxLayout* imageLayout = new QHBoxLayout();
    imageLayout->addStretch();
    imageLayout->addWidget(playerImage);
    imageLayout->addStretch();

    detailsLayout->addWidget(playerDetailsLabel);
    detailsLayout->addLayout(imageLayout);
    detailsLayout->addWidget(playerName);
    detailsLayout->addWidget(playerClub);
    detailsLayout->addWidget(playerPosition);
    detailsLayout->addWidget(playerMarketValue);
    detailsLayout->addWidget(playerRating);
    detailsLayout->addWidget(viewHistoryButton);
    detailsLayout->addStretch();
    
    playerDetailsScrollArea->setWidget(playerDetailsWidget);

    contentLayout->addWidget(tableFrame, 3);
    contentLayout->addWidget(playerDetailsScrollArea, 1);
    
    mainLayout->addLayout(contentLayout, 1);
}

void PlayerListView::setupAnimations() {
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
    connect(searchBox, &QLineEdit::textChanged, this, &PlayerListView::searchPlayers);
    connect(positionFilter, &QComboBox::currentTextChanged, this, &PlayerListView::filterByPosition);
    connect(tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, model, &PlayerListModel::sort);
    connect(backButton, &QPushButton::clicked, this, &PlayerListView::backToMain);
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
    connect(tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, 
        &PlayerListView::updatePlayerDetails);
    connect(viewHistoryButton, &QPushButton::clicked, this, &PlayerListView::showPlayerHistory);
    connect(tableView, &QTableView::doubleClicked, this, &PlayerListView::showPlayerHistory);
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
    
    if (indices.isEmpty()) return;
    
    QModelIndex index = indices.first();
    
    if (!index.isValid()) return;
    
    int playerId = model->data(model->index(index.row(), 0), Qt::DisplayRole).toInt();
    currentPlayerId = playerId;
    
    auto allPlayers = ratingManager.getSortedRatedPlayers();
    for (const auto& p : allPlayers) {
        if (p.first == playerId) {
            playerName->setText("Name: " + QString::fromStdString(p.second.name));
            playerClub->setText("Club: " + QString::fromStdString(p.second.clubName));
            playerPosition->setText("Position: " + QString::fromStdString(p.second.position) + 
                                   " (" + QString::fromStdString(p.second.subPosition) + ")");
            playerMarketValue->setText("Market Value: €" + QString::number(p.second.marketValue / 1000000.0, 'f', 1) + "M");
            playerRating->setText("Rating: " + QString::number(p.second.rating, 'f', 1));
            
            viewHistoryButton->setEnabled(true);
            
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

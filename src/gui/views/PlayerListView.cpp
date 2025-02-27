#include "gui/views/PlayerListView.h"
#include "gui/models/Models.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QParallelAnimationGroup>

PlayerListView::PlayerListView(RatingManager& rm, QWidget *parent)
    : QWidget(parent)
    , ratingManager(rm)
    , model(new PlayerListModel(ratingManager.getSortedRatedPlayers()))
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
    searchBox->setPlaceholderText("Search players...");
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

    QFrame* contentFrame = new QFrame(this);
    contentFrame->setFrameShape(QFrame::NoFrame);
    contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout* contentLayout = new QVBoxLayout(contentFrame);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(10);

    tableView = new QTableView(contentFrame);
    tableView->setObjectName("playerTable");
    tableView->setModel(model);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->horizontalHeader()->setStretchLastSection(true);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setAlternatingRowColors(true);
    tableView->setShowGrid(false);
    tableView->verticalHeader()->setVisible(false);
    
    tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    contentLayout->addWidget(tableView, 1);

    QWidget* paginationWidget = new QWidget(contentFrame);
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
    
    contentLayout->addWidget(paginationWidget, 0);
    
    mainLayout->addWidget(contentFrame, 1);
}

void PlayerListView::setupAnimations() {
    tableOpacityEffect = new QGraphicsOpacityEffect(tableView);
    tableView->setGraphicsEffect(tableOpacityEffect);
    tableOpacityEffect->setOpacity(0.0);
    
    tableOpacityAnimation = new QPropertyAnimation(tableOpacityEffect, "opacity");
    tableOpacityAnimation->setDuration(500);
    tableOpacityAnimation->setStartValue(0.0);
    tableOpacityAnimation->setEndValue(1.0);
    tableOpacityAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    tableAnimGroup = new QParallelAnimationGroup(this);
    tableAnimGroup->addAnimation(tableOpacityAnimation);
}

void PlayerListView::animateTable() {
    tableOpacityEffect->setOpacity(0.0);
    tableAnimGroup->start();
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

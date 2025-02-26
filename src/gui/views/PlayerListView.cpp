#include "gui/views/PlayerListView.h"
#include "gui/models/Models.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>

PlayerListView::PlayerListView(RatingManager& rm, QWidget *parent)
    : QWidget(parent)
    , ratingManager(rm)
    , model(new PlayerListModel(ratingManager.getSortedRatedPlayers()))
{
    setupUi();
    setupConnections();
    updatePagination();
}

void PlayerListView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    backButton = new QPushButton("Back to Menu", this);
    mainLayout->addWidget(backButton, 0, Qt::AlignLeft);

    QHBoxLayout* inputLayout = new QHBoxLayout();

    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel("Search:", this);
    searchLabel->setProperty("section", "search");
    searchBox = new QLineEdit(this);
    searchBox->setObjectName("searchBox");
    searchBox->setFixedWidth(200);
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
    positionFilter->setFixedWidth(200);
    filterLayout->addWidget(positionLabel);
    filterLayout->addWidget(positionFilter);
    filterLayout->addStretch();

    inputLayout->addLayout(searchLayout);
    inputLayout->addSpacing(20);
    inputLayout->addLayout(filterLayout);
    inputLayout->addStretch();

    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setAlternatingRowColors(true);
    tableView->setFixedWidth(665); 

    paginationLayout = new QHBoxLayout();
    prevPageButton = new QPushButton("Previous", this);
    prevPageButton->setObjectName("prevPageButton");
    nextPageButton = new QPushButton("Next", this);
    nextPageButton->setObjectName("nextPageButton");
    
    QWidget* pageInfoContainer = new QWidget(this);
    QHBoxLayout* pageInfoLayout = new QHBoxLayout(pageInfoContainer);
    pageInfoLabel = new QLabel("Page 1", this);
    pageInfoLabel->setObjectName("pageInfoLabel");
    totalPagesLabel = new QLabel("/ 1", this);
    totalPagesLabel->setObjectName("totalPagesLabel");
    
    pageInfoLayout->addStretch(1);
    pageInfoLayout->addWidget(pageInfoLabel);
    pageInfoLayout->addWidget(totalPagesLabel);
    pageInfoLayout->addStretch(1);
    pageInfoLayout->setContentsMargins(0, 0, 0, 0);

    paginationLayout->addWidget(prevPageButton);
    paginationLayout->addWidget(pageInfoContainer);
    paginationLayout->addWidget(nextPageButton);
    paginationLayout->addStretch();

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(tableView);
    mainLayout->addLayout(paginationLayout);
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
        }
    });
    connect(nextPageButton, &QPushButton::clicked, this, [this]() {
        if ((currentPage + 1) * playersPerPage < model->totalPlayers()) {
            currentPage++;
            updatePagination();
        }
    });
}

void PlayerListView::updatePagination() {
    int totalPlayers = model->totalPlayers();
    totalPages = (totalPlayers > 0) ? ((totalPlayers - 1) / playersPerPage) + 1 : 1;

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
}

void PlayerListView::filterByPosition(const QString& position) {
    currentPage = 0;
    model->setPositionFilter(position == "All Positions" ? "" : position);
    updatePagination();
}


void PlayerListView::filterPlayers() {
    searchPlayers(searchBox->text());
    filterByPosition(positionFilter->currentText());
}

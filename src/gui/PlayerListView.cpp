#include "gui/PlayerListView.h"
#include "gui/Models.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHeaderView>

PlayerListView::PlayerListView(RatingManager& rm, QWidget *parent)
    : QWidget(parent)
    , ratingManager(rm)
    , model(new PlayerListModel(ratingManager.getSortedRatedPlayers()))
{
    setupUi();
    setupConnections();
}

void PlayerListView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel("Search:", this);
    searchBox = new QLineEdit(this);
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchBox);

    QHBoxLayout* filterLayout = new QHBoxLayout();
    QLabel* positionLabel = new QLabel("Position:", this);
    positionFilter = new QComboBox(this);
    positionFilter->addItem("All Positions");
    positionFilter->addItems({"Goalkeeper", "Defender", "Midfield", "Attack"});
    filterLayout->addWidget(positionLabel);
    filterLayout->addWidget(positionFilter);

    tableView = new QTableView(this);
    tableView->setModel(model);
    tableView->setSortingEnabled(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setAlternatingRowColors(true);

    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(filterLayout);
    mainLayout->addWidget(tableView);
}

void PlayerListView::setupConnections() {
    connect(searchBox, &QLineEdit::textChanged, this, &PlayerListView::searchPlayers);
    connect(positionFilter, &QComboBox::currentTextChanged, this, &PlayerListView::filterByPosition);
}

void PlayerListView::searchPlayers(const QString& text) {
    model->setFilter(text);
}

void PlayerListView::filterByPosition(const QString& position) {
    model->setPositionFilter(position == "All Positions" ? "" : position);
}

void PlayerListView::filterPlayers() {
    searchPlayers(searchBox->text());
    filterByPosition(positionFilter->currentText());
}

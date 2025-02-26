#include "gui/components/ClubSelectDialog.h"
#include <QHeaderView>
#include <QMessageBox>

ClubSelectDialog::ClubSelectDialog(const std::vector<std::pair<int, std::string>>& clubs, QWidget* parent)
    : QDialog(parent), availableClubs(clubs)
{
    setupUi();
    setupConnections();
    updateClubsModel();
    
    resize(500, 400);
    setWindowTitle("Select Club");
}

void ClubSelectDialog::setupUi()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel("Search:", this);
    searchLineEdit = new QLineEdit(this);
    searchLineEdit->setPlaceholderText("Type to filter clubs...");
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(searchLineEdit);
    
    clubsTableView = new QTableView(this);
    clubsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    clubsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    clubsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    clubsTableView->setAlternatingRowColors(true);
    clubsTableView->horizontalHeader()->setStretchLastSection(true);
    clubsTableView->verticalHeader()->setVisible(false);
    clubsTableView->setSortingEnabled(true);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    selectButton = new QPushButton("Select", this);
    cancelButton = new QPushButton("Cancel", this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(selectButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(clubsTableView);
    mainLayout->addLayout(buttonLayout);
}

void ClubSelectDialog::setupConnections()
{
    connect(searchLineEdit, &QLineEdit::textChanged, this, &ClubSelectDialog::filterClubs);
    connect(selectButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(clubsTableView, &QTableView::doubleClicked, this, &ClubSelectDialog::handleDoubleClick);
}

void ClubSelectDialog::updateClubsModel(const QString& filter)
{
    QStandardItemModel* model = new QStandardItemModel(0, 2, this);
    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Club Name");
    
    for (const auto& [id, name] : availableClubs) {
        QString clubName = QString::fromStdString(name);
        if (filter.isEmpty() || clubName.contains(filter, Qt::CaseInsensitive)) {
            QList<QStandardItem*> row;
            row.append(new QStandardItem(QString::number(id)));
            row.append(new QStandardItem(clubName));
            model->appendRow(row);
        }
    }
    
    QAbstractItemModel* oldModel = clubsTableView->model();
    
    clubsTableView->setModel(model);
    
    clubsTableView->hideColumn(0);
    
    if (oldModel && oldModel != model) {
        oldModel->deleteLater();
    }
}

void ClubSelectDialog::filterClubs(const QString& text)
{
    updateClubsModel(text);
}

void ClubSelectDialog::handleDoubleClick(const QModelIndex& index)
{
    if (index.isValid()) {
        QModelIndex idIndex = clubsTableView->model()->index(index.row(), 0);
        selectedClubId = idIndex.data().toInt();
        accept();
    }
}

int ClubSelectDialog::getSelectedClubId() const
{
    if (selectedClubId == -1) {
        QModelIndex currentIndex = clubsTableView->currentIndex();
        if (currentIndex.isValid()) {
            QModelIndex idIndex = clubsTableView->model()->index(currentIndex.row(), 0);
            return idIndex.data().toInt();
        }
        return -1;
    }
    return selectedClubId;
}

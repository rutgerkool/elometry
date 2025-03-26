#include "gui/components/dialogs/ClubSelectDialog.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <algorithm>
#include <ranges>

ClubSelectDialog::ClubSelectDialog(std::span<const std::pair<int, std::string>> clubs, QWidget* parent)
    : QDialog(parent),
      m_availableClubs(clubs.begin(), clubs.end())
{
    setupUi();
    setupConnections();
    createModel();
    
    resize(550, 450);
    setWindowTitle(tr("Select Club"));
}

std::optional<int> ClubSelectDialog::getSelectedClubId() const {
    if (m_selectedClubId) {
        return m_selectedClubId;
    }
    
    return getCurrentClubId();
}

void ClubSelectDialog::setupUi() {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    
    auto* searchLayout = new QHBoxLayout();
    auto* searchLabel = new QLabel(tr("Search:"), this);
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText(tr("Type to filter clubs"));
    searchLayout->addWidget(searchLabel);
    searchLayout->addWidget(m_searchLineEdit, 1);
    
    m_clubsTableView = new QTableView(this);
    m_clubsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_clubsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_clubsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_clubsTableView->setAlternatingRowColors(true);
    m_clubsTableView->horizontalHeader()->setStretchLastSection(true);
    m_clubsTableView->verticalHeader()->setVisible(false);
    m_clubsTableView->setSortingEnabled(true);
    
    auto* buttonLayout = new QHBoxLayout();
    m_selectButton = new QPushButton(tr("Select"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_selectButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(m_clubsTableView, 1);
    mainLayout->addLayout(buttonLayout);
}

void ClubSelectDialog::setupConnections() {
    connect(m_searchLineEdit, &QLineEdit::textChanged, this, &ClubSelectDialog::filterClubs);
    connect(m_selectButton, &QPushButton::clicked, this, &ClubSelectDialog::handleClubSelection);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_clubsTableView, &QTableView::doubleClicked, this, &ClubSelectDialog::handleDoubleClick);
}

void ClubSelectDialog::createModel() {
    m_clubsModel = createFilteredModel({});
    m_clubsTableView->setModel(m_clubsModel);
    m_clubsTableView->hideColumn(0);
}

void ClubSelectDialog::updateModel(const QString& filter) {
    auto* oldModel = m_clubsModel;
    m_clubsModel = createFilteredModel(filter);
    m_clubsTableView->setModel(m_clubsModel);
    m_clubsTableView->hideColumn(0);
    
    delete oldModel;
}

QStandardItemModel* ClubSelectDialog::createFilteredModel(const QString& filter) const {
    auto* model = new QStandardItemModel(0, 2, const_cast<ClubSelectDialog*>(this));
    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Club Name"));
    
    for (const auto& [id, name] : m_availableClubs) {
        QString clubName = QString::fromStdString(name);
        if (filter.isEmpty() || clubName.contains(filter, Qt::CaseInsensitive)) {
            QList<QStandardItem*> row;
            row.append(new QStandardItem(QString::number(id)));
            row.append(new QStandardItem(clubName));
            model->appendRow(row);
        }
    }
    
    return model;
}

int ClubSelectDialog::getCurrentClubId() const {
    const QModelIndex currentIndex = m_clubsTableView->currentIndex();
    if (!currentIndex.isValid()) {
        return -1;
    }
    
    const QModelIndex idIndex = m_clubsModel->index(currentIndex.row(), 0);
    return idIndex.data().toInt();
}

void ClubSelectDialog::filterClubs() {
    updateModel(m_searchLineEdit->text().trimmed());
}

void ClubSelectDialog::handleClubSelection() {
    int clubId = getCurrentClubId();
    if (clubId != -1) {
        m_selectedClubId = clubId;
        accept();
    }
}

void ClubSelectDialog::handleDoubleClick(const QModelIndex& index) {
    if (index.isValid()) {
        const QModelIndex idIndex = m_clubsTableView->model()->index(index.row(), 0);
        m_selectedClubId = idIndex.data().toInt();
        accept();
    }
}

#ifndef PLAYERLISTVIEW_H
#define PLAYERLISTVIEW_H

#include "services/RatingManager.h"
#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>

class PlayerListModel;

class PlayerListView : public QWidget {
    Q_OBJECT

    public:
        explicit PlayerListView(RatingManager& ratingManager, QWidget *parent = nullptr);

    signals:
        void backToMain(); 

    private slots:
        void filterPlayers();
        void searchPlayers(const QString& text);
        void filterByPosition(const QString& position);

    private:
        int currentPage = 0;
        int totalPages = 1;
        const int playersPerPage = 20;

        QPushButton* prevPageButton;
        QPushButton* nextPageButton;
        QPushButton* backButton;
        QLabel* pageInfoLabel;
        QTableView* tableView;
        QLineEdit* searchBox;
        QComboBox* positionFilter;
        QHBoxLayout* paginationLayout;
        QLabel* totalPagesLabel;

        RatingManager& ratingManager;
        PlayerListModel* model;

        void setupUi();
        void setupConnections();
        void updatePagination();
};

#endif

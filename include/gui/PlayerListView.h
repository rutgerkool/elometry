#ifndef PLAYERLISTVIEW_H
#define PLAYERLISTVIEW_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include "services/RatingManager.h"

class PlayerListModel;

class PlayerListView : public QWidget {
    Q_OBJECT

public:
    explicit PlayerListView(RatingManager& ratingManager, QWidget *parent = nullptr);

private slots:
    void filterPlayers();
    void searchPlayers(const QString& text);
    void filterByPosition(const QString& position);

private:
    void setupUi();
    void setupConnections();

    RatingManager& ratingManager;
    PlayerListModel* model;
    QTableView* tableView;
    QLineEdit* searchBox;
    QComboBox* positionFilter;
};

#endif

#ifndef PLAYERLISTMODEL_H
#define PLAYERLISTMODEL_H

#include "models/PlayerRating.h"
#include "services/TeamManager.h"
#include <QtCore/QAbstractTableModel>
#include <QtCore/QAbstractListModel>
#include <QtCore/QString>
#include <vector>

class PlayerListModel : public QAbstractTableModel {
    Q_OBJECT

    public:
        explicit PlayerListModel(const std::vector<std::pair<int, Player>>& players, QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        void setPagination(int start, int max);
        size_t totalPlayers() const;
        int filteredPlayerCount() const;  

    public slots:
        void setFilter(const QString& filter);
        void setPositionFilter(const QString& position);
        void sort(int column, Qt::SortOrder order);
        
    private:
        int startIndex = 0;
        int maxPlayers = 20;
        std::vector<std::pair<int, Player>> allPlayers;
        std::vector<std::pair<int, Player>> filteredPlayers;
        QString currentFilter;
        QString currentPosition;
};

#endif

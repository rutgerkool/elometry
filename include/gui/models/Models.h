#ifndef MODELS_H
#define MODELS_H

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

class TeamListModel : public QAbstractListModel {
    Q_OBJECT

    public:
        explicit TeamListModel(TeamManager& teamManager, QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    public slots:
        void refresh();

    private:
        TeamManager& teamManager;
        std::vector<Team> teams;
};

#endif

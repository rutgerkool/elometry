#include "gui/Models.h"
#include <QtCore/QString>
#include <algorithm>

PlayerListModel::PlayerListModel(const std::vector<std::pair<int, Player>>& players, QObject *parent)
    : QAbstractTableModel(parent)
    , allPlayers(players)
    , filteredPlayers(players)
{
}

int PlayerListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return filteredPlayers.size();
}

int PlayerListModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 5; 
}

QVariant PlayerListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        const auto& player = filteredPlayers[index.row()];
        switch (index.column()) {
            case 0: return player.first; 
            case 1: return QString::fromStdString(player.second.name);
            case 2: return player.second.rating;
            case 3: return QString::fromStdString(player.second.subPosition);
            case 4: return player.second.marketValue;
        }
    }

    return QVariant();
}

QVariant PlayerListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0: return "ID";
            case 1: return "Name";
            case 2: return "Rating";
            case 3: return "Position";
            case 4: return "Market Value";
        }
    }

    return QVariant();
}

void PlayerListModel::setFilter(const QString& filter) {
    beginResetModel();
    currentFilter = filter;
    
    filteredPlayers = allPlayers;
    if (!filter.isEmpty()) {
        filteredPlayers.erase(
            std::remove_if(filteredPlayers.begin(), filteredPlayers.end(),
                [&filter](const auto& player) {
                    return !QString::fromStdString(player.second.name)
                        .contains(filter, Qt::CaseInsensitive);
                }
            ),
            filteredPlayers.end()
        );
    }

    if (!currentPosition.isEmpty()) {
        filteredPlayers.erase(
            std::remove_if(filteredPlayers.begin(), filteredPlayers.end(),
                [this](const auto& player) {
                    return player.second.position != currentPosition.toStdString();
                }
            ),
            filteredPlayers.end()
        );
    }
    endResetModel();
}

void PlayerListModel::setPositionFilter(const QString& position) {
    currentPosition = position;
    setFilter(currentFilter); 
}

TeamListModel::TeamListModel(TeamManager& tm, QObject *parent)
    : QAbstractListModel(parent)
    , teamManager(tm)
{
    refresh();
}

int TeamListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return teams.size();
}

QVariant TeamListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        return QString::fromStdString(teams[index.row()].teamName);
    }
    else if (role == Qt::UserRole) {
        return teams[index.row()].teamId;
    }

    return QVariant();
}

void TeamListModel::refresh() {
    beginResetModel();
    teams = teamManager.getAllTeams();
    endResetModel();
}

#include "gui/models/PlayerListModel.h"
#include <algorithm>

PlayerListModel::PlayerListModel(const std::vector<std::pair<int, Player>>& players, QObject *parent)
    : QAbstractTableModel(parent)
    , allPlayers(players)
    , filteredPlayers(players)
{
}

int PlayerListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    
    int totalFiltered = filteredPlayerCount();
    
    return std::min(maxPlayers, totalFiltered - startIndex);
}

int PlayerListModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 5; 
}

QVariant PlayerListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    
    int actualRow = index.row() + startIndex;
    
    if (actualRow < 0 || static_cast<size_t>(actualRow) >= filteredPlayers.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        const auto& player = filteredPlayers[actualRow];
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

int PlayerListModel::filteredPlayerCount() const {
    return static_cast<int>(filteredPlayers.size());
}

size_t PlayerListModel::totalPlayers() const {
    return allPlayers.size();
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
    
    startIndex = 0;
    
    endResetModel();
}

void PlayerListModel::setPagination(int start, int max) {
    beginResetModel();
    startIndex = start;
    maxPlayers = max;
    endResetModel();
}


void PlayerListModel::setPositionFilter(const QString& position) {
    currentPosition = position;
    setFilter(currentFilter);
}

void PlayerListModel::sort(int column, Qt::SortOrder order) {
    beginResetModel();

    std::sort(filteredPlayers.begin(), filteredPlayers.end(),
        [column, order](const std::pair<int, Player>& a, const std::pair<int, Player>& b) {
            switch (column) {
                case 0: return order == Qt::AscendingOrder ? a.first < b.first : a.first > b.first;  
                case 1: return order == Qt::AscendingOrder ? a.second.name < b.second.name : a.second.name > b.second.name;  
                case 2: return order == Qt::AscendingOrder ? a.second.rating < b.second.rating : a.second.rating > b.second.rating;  
                case 3: return order == Qt::AscendingOrder ? a.second.subPosition < b.second.subPosition : a.second.subPosition > b.second.subPosition;
                case 4: return order == Qt::AscendingOrder ? a.second.marketValue < b.second.marketValue : a.second.marketValue > b.second.marketValue;
            }
            return false;
        });
        
    startIndex = 0;

    endResetModel();
}

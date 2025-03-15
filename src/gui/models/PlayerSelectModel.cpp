#include "gui/models/PlayerSelectModel.h"
#include <QtWidgets/QMessageBox>
#include <QtGui/QStandardItem>
#include <QtGui/QIcon>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QTimer>
#include <algorithm>

PlayerSelectModel::PlayerSelectModel(const std::vector<std::pair<int, Player>>& players, QObject *parent)
    : QAbstractTableModel(parent)
    , allPlayers(players)
    , filteredPlayers(players)
{
}

int PlayerSelectModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    
    int totalFiltered = filteredPlayerCount();
    return std::min(maxPlayers, totalFiltered - startIndex);
}

int PlayerSelectModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return 6;
}

QVariant PlayerSelectModel::getDisplayData(const QModelIndex &index) const {
    const auto& player = filteredPlayers[index.row() + startIndex].second;
    
    switch (index.column()) {
        case 0: return QVariant();
        case 1: return filteredPlayers[index.row() + startIndex].first;
        case 2: return QString::fromStdString(player.name);
        case 3: return player.rating;
        case 4: return QString::fromStdString(player.subPosition);
        case 5: {
            double valueInMillions = player.marketValue / 1000000.0;
            return QString("€%1M").arg(valueInMillions, 0, 'f', 1);
        }
        default: return QVariant();
    }
}

QVariant PlayerSelectModel::getDecorativeData(const QModelIndex &index) const {
    int playerId = filteredPlayers[index.row() + startIndex].first;
    bool selected = isPlayerSelected(playerId);
    
    if (index.column() == 0 && index.data(Qt::DisplayRole).isNull()) {
        return selected ? Qt::Checked : Qt::Unchecked;
    }
    
    if (selected) {
        return QColor(45, 65, 90);
    }
    
    return (index.row() % 2) ? QColor(45, 45, 45) : QColor(53, 53, 53);
}

QVariant PlayerSelectModel::getAlignmentData(const QModelIndex &index) const {
    if (index.column() == 0) {
        return int(Qt::AlignCenter);
    }
    else if (index.column() == 3 || index.column() == 5) {
        return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return int(Qt::AlignLeft | Qt::AlignVCenter);
}

QVariant PlayerSelectModel::getTooltipData(const QModelIndex &index) const {
    int playerId = filteredPlayers[index.row() + startIndex].first;
    bool selected = isPlayerSelected(playerId);
    const auto& player = filteredPlayers[index.row() + startIndex].second;
    
    if (index.column() == 0) {
        return selected ? "Click to unselect" : "Click to select";
    }
    else if (index.column() == 5) {
        return QString("€%L1").arg(player.marketValue);
    }
    else if (index.column() == 2) {
        return QString::fromStdString(player.name);
    }
    
    return QVariant();
}

QVariant PlayerSelectModel::getFontData(const QModelIndex &index) const {
    if (index.column() == 2) {
        QFont font;
        font.setBold(true);
        return font;
    }
    return QVariant();
}

QVariant PlayerSelectModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    
    int actualRow = index.row() + startIndex;
    if (actualRow < 0 || actualRow >= filteredPlayers.size()) return QVariant();

    int playerId = filteredPlayers[actualRow].first;
    bool selected = isPlayerSelected(playerId);

    switch (role) {
        case Qt::DisplayRole:
            return getDisplayData(index);
        case Qt::CheckStateRole:
            return (index.column() == 0) ? (selected ? Qt::Checked : Qt::Unchecked) : QVariant();
        case Qt::BackgroundRole:
            return selected ? QColor(45, 65, 90) : ((index.row() % 2) ? QColor(45, 45, 45) : QColor(53, 53, 53));
        case Qt::ForegroundRole:
            return selected ? QColor(220, 220, 220) : QColor(210, 210, 210);
        case Qt::UserRole:
            return playerId;
        case Qt::TextAlignmentRole:
            return getAlignmentData(index);
        case Qt::ToolTipRole:
            return getTooltipData(index);
        case Qt::FontRole:
            return getFontData(index);
        default:
            return QVariant();
    }
}

QVariant PlayerSelectModel::getHeaderDisplayData(int section) const {
    switch (section) {
        case 0: return "";
        case 1: return "ID";
        case 2: return "Name";
        case 3: return "Rating";
        case 4: return "Position";
        case 5: return "Market Value";
        default: return QVariant();
    }
}

QVariant PlayerSelectModel::getHeaderTooltipData(int section) const {
    switch (section) {
        case 0: return "Select/Unselect";
        case 1: return "Player ID";
        case 2: return "Player Name";
        case 3: return "Player Rating";
        case 4: return "Player Position";
        case 5: return "Player Market Value";
        default: return QVariant();
    }
}

QVariant PlayerSelectModel::getHeaderAlignmentData(int section) const {
    if (section == 0) {
        return int(Qt::AlignCenter);
    }
    else if (section == 3 || section == 5) {
        return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return int(Qt::AlignLeft | Qt::AlignVCenter);
}

QVariant PlayerSelectModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal) return QVariant();
    
    switch (role) {
        case Qt::DisplayRole:
            return getHeaderDisplayData(section);
        case Qt::ForegroundRole:
            return QColor(220, 220, 220);
        case Qt::TextAlignmentRole:
            return getHeaderAlignmentData(section);
        case Qt::ToolTipRole:
            return getHeaderTooltipData(section);
        default:
            return QVariant();
    }
}

bool PlayerSelectModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (!index.isValid() || index.column() != 0 || role != Qt::CheckStateRole) 
        return false;
    
    int actualRow = index.row() + startIndex;
    if (actualRow >= filteredPlayers.size())
        return false;
    
    int playerId = filteredPlayers[actualRow].first;
    if (value == Qt::Checked) {
        selectedPlayerIds.insert(playerId);
    } else {
        selectedPlayerIds.erase(playerId);
    }
    
    emit dataChanged(this->index(index.row(), 0), this->index(index.row(), columnCount() - 1));
    return true;
}

Qt::ItemFlags PlayerSelectModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    
    if (index.column() == 0) {
        flags |= Qt::ItemIsUserCheckable;
    }
    
    return flags;
}

void PlayerSelectModel::applyFilters() {
    filteredPlayers = allPlayers;
    
    if (!currentFilter.isEmpty()) {
        filteredPlayers.erase(
            std::remove_if(filteredPlayers.begin(), filteredPlayers.end(),
                [this](const auto& player) {
                    return !QString::fromStdString(player.second.name)
                        .contains(currentFilter, Qt::CaseInsensitive);
                }
            ),
            filteredPlayers.end()
        );
    }

    if (!currentPosition.isEmpty()) {
        filteredPlayers.erase(
            std::remove_if(filteredPlayers.begin(), filteredPlayers.end(),
                [this](const auto& player) {
                    return player.second.subPosition != currentPosition.toStdString() && 
                           player.second.position != currentPosition.toStdString();
                }
            ),
            filteredPlayers.end()
        );
    }
}

void PlayerSelectModel::setFilter(const QString& filter) {
    beginResetModel();
    currentFilter = filter;
    applyFilters();
    startIndex = 0;
    endResetModel();
}

void PlayerSelectModel::setPositionFilter(const QString& position) {
    beginResetModel();
    currentPosition = position;
    applyFilters();
    startIndex = 0;
    endResetModel();
}

void PlayerSelectModel::setPagination(int start, int max) {
    beginResetModel();
    startIndex = start;
    maxPlayers = max;
    endResetModel();
}

int PlayerSelectModel::filteredPlayerCount() const {
    return static_cast<int>(filteredPlayers.size());
}

void PlayerSelectModel::togglePlayerSelection(const QModelIndex &index) {
    if (!index.isValid()) return;
    
    int actualRow = index.row() + startIndex;
    if (actualRow >= filteredPlayers.size()) return;
    
    int playerId = filteredPlayers[actualRow].first;
    
    if (isPlayerSelected(playerId)) {
        selectedPlayerIds.erase(playerId);
    } else {
        selectedPlayerIds.insert(playerId);
    }
    
    emit dataChanged(this->index(index.row(), 0), this->index(index.row(), columnCount() - 1));
}

void PlayerSelectModel::updatePlayerVisibility(int playerId) {
    for (size_t i = 0; i < filteredPlayers.size(); ++i) {
        if (filteredPlayers[i].first == playerId) {
            int visibleRow = static_cast<int>(i) - startIndex;
            if (visibleRow >= 0 && visibleRow < maxPlayers) {
                emit dataChanged(this->index(visibleRow, 0), 
                                this->index(visibleRow, columnCount() - 1));
            }
            break;
        }
    }
}

void PlayerSelectModel::selectPlayer(int playerId) {
    selectedPlayerIds.insert(playerId);
    updatePlayerVisibility(playerId);
}

void PlayerSelectModel::deselectPlayer(int playerId) {
    selectedPlayerIds.erase(playerId);
    updatePlayerVisibility(playerId);
}

bool PlayerSelectModel::isPlayerSelected(int playerId) const {
    return selectedPlayerIds.find(playerId) != selectedPlayerIds.end();
}

std::vector<Player> PlayerSelectModel::getSelectedPlayers() const {
    std::vector<Player> selectedPlayers;
    selectedPlayers.reserve(selectedPlayerIds.size());
    
    for (const auto& pair : allPlayers) {
        if (isPlayerSelected(pair.first)) {
            selectedPlayers.push_back(pair.second);
        }
    }
    
    return selectedPlayers;
}

int PlayerSelectModel::getSelectedCount() const {
    return static_cast<int>(selectedPlayerIds.size());
}

void PlayerSelectModel::sort(int column, Qt::SortOrder order) {
    beginResetModel();

    std::sort(filteredPlayers.begin(), filteredPlayers.end(),
        [column, order, this](const std::pair<int, Player>& a, const std::pair<int, Player>& b) {
            if (column == 0) {
                bool aSelected = isPlayerSelected(a.first);
                bool bSelected = isPlayerSelected(b.first);
                return order == Qt::AscendingOrder ? aSelected < bSelected : aSelected > bSelected;
            }
            
            switch (column) {
                case 1: return order == Qt::AscendingOrder ? a.first < b.first : a.first > b.first;  
                case 2: return order == Qt::AscendingOrder ? a.second.name < b.second.name : a.second.name > b.second.name;  
                case 3: return order == Qt::AscendingOrder ? a.second.rating < b.second.rating : a.second.rating > b.second.rating;  
                case 4: return order == Qt::AscendingOrder ? a.second.subPosition < b.second.subPosition : a.second.subPosition > b.second.subPosition;
                case 5: return order == Qt::AscendingOrder ? a.second.marketValue < b.second.marketValue : a.second.marketValue > b.second.marketValue;
                default: return false;
            }
        });
        
    startIndex = 0;
    endResetModel();
}

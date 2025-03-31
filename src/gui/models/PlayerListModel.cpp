#include "gui/models/PlayerListModel.h"
#include <algorithm>
#include <ranges>

PlayerListModel::PlayerListModel(std::span<const std::pair<int, Player>> players, QObject* parent)
    : QAbstractTableModel(parent)
    , m_allPlayers(players.begin(), players.end())
    , m_filteredPlayers(players.begin(), players.end())
{
}

int PlayerListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    
    const int totalFiltered = filteredPlayerCount();
    return std::min(m_maxPlayers, totalFiltered - m_startIndex);
}

int PlayerListModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : 5;
}

QVariant PlayerListModel::getDisplayData(const QModelIndex& index) const {
    const int actualRow = index.row() + m_startIndex;
    
    if (actualRow < 0 || actualRow >= static_cast<int>(m_filteredPlayers.size())) {
        return {};
    }

    const auto& [playerId, player] = m_filteredPlayers[actualRow];
    
    switch (index.column()) {
        case 0: return playerId;
        case 1: return QString::fromStdString(player.name);
        case 2: return player.rating;
        case 3: return QString::fromStdString(player.subPosition);
        case 4: return player.marketValue;
        default: return {};
    }
}

QVariant PlayerListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return {};
    }
    
    return role == Qt::DisplayRole ? getDisplayData(index) : QVariant{};
}

QVariant PlayerListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return {};
    }

    switch (section) {
        case 0: return "ID";
        case 1: return "Name";
        case 2: return "Rating";
        case 3: return "Position";
        case 4: return "Market Value";
        default: return {};
    }
}

bool PlayerListModel::matchesFilter(const std::pair<int, Player>& player) const {
    if (m_currentFilter.isEmpty()) {
        return true;
    }
    
    return QString::fromStdString(player.second.name)
        .contains(m_currentFilter, Qt::CaseInsensitive);
}

bool PlayerListModel::matchesPosition(const std::pair<int, Player>& player) const {
    if (m_currentPosition.isEmpty()) {
        return true;
    }
    
    return player.second.position == m_currentPosition.toStdString();
}
void PlayerListModel::applyFilters() {
    auto filteredView = m_allPlayers 
        | std::views::filter([this](const auto& player) {
            return matchesFilter(player) && matchesPosition(player);
        });
    
    m_filteredPlayers.assign(filteredView.begin(), filteredView.end());
}

int PlayerListModel::filteredPlayerCount() const noexcept {
    return static_cast<int>(m_filteredPlayers.size());
}

size_t PlayerListModel::totalPlayers() const noexcept {
    return m_allPlayers.size();
}

void PlayerListModel::setPagination(int start, int max) {
    beginResetModel();
    m_startIndex = start;
    m_maxPlayers = max;
    endResetModel();
}

void PlayerListModel::setFilter(const QString& filter) {
    beginResetModel();
    m_currentFilter = filter;
    
    applyFilters();
    m_startIndex = 0;
    
    endResetModel();
}

void PlayerListModel::setPositionFilter(const QString& position) {
    m_currentPosition = position;
    setFilter(m_currentFilter);
}

void PlayerListModel::sort(int column, Qt::SortOrder order) {
    beginResetModel();

    auto comparator = [column, order](const auto& a, const auto& b) {
        const bool isAscending = order == Qt::AscendingOrder;
        
        switch (column) {
            case 0: return isAscending ? a.first < b.first : a.first > b.first;
            case 1: return isAscending ? a.second.name < b.second.name : a.second.name > b.second.name;
            case 2: return isAscending ? a.second.rating < b.second.rating : a.second.rating > b.second.rating;
            case 3: return isAscending ? a.second.subPosition < b.second.subPosition : a.second.subPosition > b.second.subPosition;
            case 4: return isAscending ? a.second.marketValue < b.second.marketValue : a.second.marketValue > b.second.marketValue;
            default: return false;
        }
    };
    
    std::ranges::sort(m_filteredPlayers, comparator);
    m_startIndex = 0;

    endResetModel();
}
